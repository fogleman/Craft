from contextlib import contextmanager
from math import floor
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
import Queue
import SocketServer
import datetime
import random
import re
import sys
import threading
import traceback
import zlib

HOST = '0.0.0.0'
PORT = 4080
CHUNK_SIZE = 32
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

def chunked(x):
    return int(floor(round(x) / CHUNK_SIZE))

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
                buf = []
                try:
                    buf.append(self.queue.get(timeout=5))
                    try:
                        while True:
                            buf.append(self.queue.get(False))
                    except Queue.Empty:
                        pass
                except Queue.Empty:
                    continue
                data = ''.join(buf)
                self.request.sendall(data)
            except Exception:
                self.request.close()
                raise
    def send_raw(self, data):
        if data:
            self.queue.put(zlib.compress(data))
    def send(self, *args):
        data = '%s\n' % ','.join(map(str, args))
        #log('SEND', self.client_id, data[:-1])
        self.send_raw(data)

class Model(object):
    def __init__(self):
        self.next_client_id = 1
        self.clients = []
        self.queue = Queue.Queue()
        self.commands = {
            CHUNK: self.on_chunk,
            BLOCK: self.on_block,
            POSITION: self.on_position,
            TALK: self.on_talk,
        }
        self.patterns = [
            (re.compile(r'^/nick(?:\s+(\S+))?$'), self.on_nick),
            (re.compile(r'^/spawn$'), self.on_spawn),
            (re.compile(r'^/goto(?:\s+(\S+))?$'), self.on_goto),
            (re.compile(r'^/help$'), self.on_help),
            (re.compile(r'^/players$'), self.on_players),
        ]
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
        client.nick = 'player%d' % client.client_id
        self.next_client_id += 1
        log('CONN', client.client_id, *client.client_address)
        self.spawn(client)
        self.clients.append(client)
        client.send(YOU, client.client_id, *client.position)
        client.send(TALK, 'Welcome to Craft!')
        client.send(TALK, 'Type "/help" for chat commands.')
        self.send_position(client)
        self.send_positions(client)
        self.send_talk(client,
            '%s has joined the game.' % client.nick)
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
        self.send_talk(client,
            '%s has disconnected from the server.' % client.nick)
    def on_chunk(self, client, p, q):
        p, q = map(int, (p, q))
        with session() as sql:
            query = (
                'select x, y, z, w from block where '
                'p = :p and q = :q;'
            )
            rows = sql.execute(query, dict(p=p, q=q))
            buf = []
            for x, y, z, w in rows:
                args = (BLOCK, p, q, x, y, z, w)
                buf.append('%s\n' % ','.join(map(str, args)))
            client.send_raw(''.join(buf))
    def on_block(self, client, x, y, z, w):
        x, y, z, w = map(int, (x, y, z, w))
        if y <= 0 or y > 255 or w < 0 or w > 11:
            return
        p, q = chunked(x), chunked(z)
        with session() as sql:
            query = (
                'insert or replace into block (p, q, x, y, z, w) '
                'values (:p, :q, :x, :y, :z, :w);'
            )
            sql.execute(query, dict(p=p, q=q, x=x, y=y, z=z, w=w))
            self.send_block(client, p, q, x, y, z, w)
            if chunked(x - 1) != p:
                sql.execute(query, dict(p=p - 1, q=q, x=x, y=y, z=z, w=-w))
                self.send_block(client, p - 1, q, x, y, z, -w)
            if chunked(x + 1) != p:
                sql.execute(query, dict(p=p + 1, q=q, x=x, y=y, z=z, w=-w))
                self.send_block(client, p + 1, q, x, y, z, -w)
            if chunked(z - 1) != q:
                sql.execute(query, dict(p=p, q=q - 1, x=x, y=y, z=z, w=-w))
                self.send_block(client, p, q - 1, x, y, z, -w)
            if chunked(z + 1) != q:
                sql.execute(query, dict(p=p, q=q + 1, x=x, y=y, z=z, w=-w))
                self.send_block(client, p, q + 1, x, y, z, -w)
    def on_position(self, client, x, y, z, rx, ry):
        x, y, z, rx, ry = map(float, (x, y, z, rx, ry))
        client.position = (x, y, z, rx, ry)
        self.send_position(client)
    def on_talk(self, client, *args):
        text = ','.join(args)
        if text.startswith('/'):
            for pattern, func in self.patterns:
                match = pattern.match(text)
                if match:
                    func(client, *match.groups())
        else:
            text = '%s> %s' % (client.nick, text)
            self.send_talk(client, text)
    def on_nick(self, client, nick=None):
        if nick is None:
            client.send(TALK, 'Your nickname is %s' % client.nick)
        else:
            self.send_talk(client,
                '%s is now known as %s' % (client.nick, nick))
            client.nick = nick
    def on_spawn(self, client):
        self.spawn(client)
        client.send(YOU, client.client_id, *client.position)
        self.send_position(client)
    def on_goto(self, client, nick=None):
        if nick is None:
            clients = [x for x in self.clients if x != client]
            other = random.choice(self.clients) if clients else None
        else:
            nicks = dict((client.nick, client) for client in self.clients)
            other = nicks.get(nick)
        if other:
            client.position = other.position
            client.send(YOU, client.client_id, *client.position)
            self.send_position(client)
    def on_help(self, client):
        client.send(TALK, 'Type "t" to chat with other players.')
        client.send(TALK, 'Type "/" to start typing a command.')
        client.send(TALK,
            'Commands: /goto [NAME], /help, /nick [NAME], /players, /spawn')
    def on_players(self, client):
        client.send(TALK,
            'Players: %s' % ', '.join(x.nick for x in self.clients))
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
