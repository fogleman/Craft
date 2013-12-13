from contextlib import contextmanager
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
import Queue
import SocketServer
import datetime
import sys
import threading
import traceback

HOST = '0.0.0.0'
PORT = 4080
BUFFER_SIZE = 1024
ENGINE = 'sqlite:///craft.db'
SPAWN_POINT = None

YOU = 'U'
BLOCK = 'B'
CHUNK = 'C'
POSITION = 'P'
DISCONNECT = 'D'
TALK = 'T'

Session = sessionmaker(bind=create_engine(ENGINE))

@contextmanager
def session():
    session = Session()
    try:
        yield session
        session.commit()
    except:
        session.rollback()
        raise
    finally:
        session.close()

def log(*args):
    now = datetime.datetime.utcnow()
    print now, ' '.join(map(str, args))

class Server(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
    allow_reuse_address = True
    daemon_threads = True

class Handler(SocketServer.BaseRequestHandler):
    def setup(self):
        self.queue = Queue.Queue()
        self.running = True
        self.start()
    def handle(self):
        model = self.server.model
        model.enqueue(model.on_connect, self)
        try:
            buf = []
            while True:
                data = self.request.recv(BUFFER_SIZE)
                if not data:
                    break
                buf.extend(data.replace('\r', ''))
                while '\n' in buf:
                    index = buf.index('\n')
                    line = ''.join(buf[:index])
                    buf = buf[index + 1:]
                    model.enqueue(model.on_data, self, line)
        finally:
            model.enqueue(model.on_disconnect, self)
    def finish(self):
        self.running = False
    def start(self):
        thread = threading.Thread(target=self.run)
        thread.setDaemon(True)
        thread.start()
    def run(self):
        while self.running:
            try:
                try:
                    data = self.queue.get(timeout=5)
                except Queue.Empty:
                    continue
                self.request.sendall(data)
            except Exception:
                self.request.close()
                raise
    def send(self, *args):
        data = ','.join(str(x) for x in args)
        #log('SEND', self.client_id, data)
        data = '%s\n' % data
        self.queue.put(data)

class Model(object):
    def __init__(self):
        self.next_client_id = 0
        self.clients = []
        self.queue = Queue.Queue()
        self.commands = {
            CHUNK: self.on_chunk,
            BLOCK: self.on_block,
            POSITION: self.on_position,
            TALK: self.on_talk,
        }
    def start(self):
        thread = threading.Thread(target=self.run)
        thread.setDaemon(True)
        thread.start()
    def run(self):
        while True:
            try:
                self.dequeue()
            except Exception:
                traceback.print_exc()
    def enqueue(self, func, *args, **kwargs):
        self.queue.put((func, args, kwargs))
    def dequeue(self):
        func, args, kwargs = self.queue.get()
        func(*args, **kwargs)
    def on_connect(self, client):
        client.client_id = self.next_client_id
        self.next_client_id += 1
        log('CONN', client.client_id, *client.client_address)
        self.spawn(client)
        self.clients.append(client)
        client.send(YOU, client.client_id, *client.position)
        client.send(TALK, 'Welcome to Craft!')
        self.send_position(client)
        self.send_positions(client)
    def on_data(self, client, data):
        #log('RECV', client.client_id, data)
        args = data.split(',')
        command, args = args[0], args[1:]
        if command in self.commands:
            func = self.commands[command]
            func(client, *args)
    def on_disconnect(self, client):
        log('DISC', client.client_id, *client.client_address)
        self.clients.remove(client)
        self.send_disconnect(client)
    def on_chunk(self, client, p, q):
        p, q = map(int, (p, q))
        with session() as sql:
            query = (
                'select x, y, z, w from block where w >= 0 and '
                'p = :p and q = :q;'
            )
            rows = sql.execute(query, dict(p=p, q=q))
            for x, y, z, w in rows:
                client.send(BLOCK, p, q, x, y, z, w)
    def on_block(self, client, p, q, x, y, z, w):
        p, q, x, y, z, w = map(int, (p, q, x, y, z, w))
        if y <= 0 or w < -1 or w > 11:
            return
        with session() as sql:
            query = (
                'insert or replace into block (p, q, x, y, z, w) '
                'values (:p, :q, :x, :y, :z, :w);'
            )
            sql.execute(query, dict(p=p, q=q, x=x, y=y, z=z, w=w))
        if w >= 0:
            self.send_block(client, p, q, x, y, z, w)
    def on_position(self, client, x, y, z, rx, ry):
        x, y, z, rx, ry = map(float, (x, y, z, rx, ry))
        client.position = (x, y, z, rx, ry)
        self.send_position(client)
    def on_talk(self, client, text):
        if text.startswith('/'):
            pass
        else:
            self.send_talk(client, text)
    def send_positions(self, client):
        for other in self.clients:
            if other == client:
                continue
            client.send(POSITION, other.client_id, *other.position)
    def send_position(self, client):
        for other in self.clients:
            if other == client:
                continue
            other.send(POSITION, client.client_id, *client.position)
    def send_disconnect(self, client):
        for other in self.clients:
            if other == client:
                continue
            other.send(DISCONNECT, client.client_id)
    def send_block(self, client, p, q, x, y, z, w):
        for other in self.clients:
            if other == client:
                continue
            other.send(BLOCK, p, q, x, y, z, w)
    def send_talk(self, client, text):
        for other in self.clients:
            if other == client:
                continue
            other.send(TALK, text)
    def spawn(self, client):
        if SPAWN_POINT is not None:
            client.position = SPAWN_POINT
        else:
            with session() as sql:
                query = 'select x, y, z from block order by random() limit 1;'
                rows = list(sql.execute(query))
                if rows:
                    x, y, z = rows[0]
                    client.position = (x, y, z, 0, 0)
                else:
                    client.position = (0, 0, 0, 0, 0)

def main():
    queries = [
        'create table if not exists block ('
        '    p int not null,'
        '    q int not null,'
        '    x int not null,'
        '    y int not null,'
        '    z int not null,'
        '    w int not null'
        ');',
        'create index if not exists block_xyz_idx on block (x, y, z);',
        'create unique index if not exists block_pqxyz_idx on '
        '    block (p, q, x, y, z);',
    ]
    with session() as sql:
        for query in queries:
            sql.execute(query)
    host, port = HOST, PORT
    if len(sys.argv) > 1:
        host = sys.argv[1]
    if len(sys.argv) > 2:
        port = int(sys.argv[2])
    log('SERV', host, port)
    model = Model()
    model.start()
    server = Server((host, port), Handler)
    server.model = model
    server.serve_forever()

if __name__ == '__main__':
    main()
