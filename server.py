from math import floor
import Queue
import SocketServer
import datetime
import random
import re
import sqlite3
import sys
import threading
import time
import traceback

HOST = '0.0.0.0'
PORT = 4080
CHUNK_SIZE = 32
BUFFER_SIZE = 1024
SPAWN_POINT = (0, 0, 0, 0, 0)
DB_PATH = 'craft.db'
COMMIT_INTERVAL = 5
INVENTORY_SLOTS = 9

YOU = 'U'
BLOCK = 'B'
CHUNK = 'C'
POSITION = 'P'
DISCONNECT = 'D'
TALK = 'T'
KEY = 'K'
INVENTORY = 'I'

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
            self.queue.put(data)
    def send(self, *args):
        data = '%s\n' % ','.join(map(str, args))
        #log('SEND', self.client_id, data[:-1])
        self.send_raw(data)

class Model(object):
    def __init__(self):
        self.clients = []
        self.queue = Queue.Queue()
        self.commands = {
            CHUNK: self.on_chunk,
            BLOCK: self.on_block,
            POSITION: self.on_position,
            TALK: self.on_talk,
            INVENTORY: self.on_inventory,
        }
        self.patterns = [
            (re.compile(r'^/nick(?:\s+(\S+))?$'), self.on_nick),
            (re.compile(r'^/spawn$'), self.on_spawn),
            (re.compile(r'^/goto(?:\s+(\S+))?$'), self.on_goto),
            (re.compile(r'^/pq\s+(-?[0-9]+)\s*,?\s*(-?[0-9]+)$'), self.on_pq),
            (re.compile(r'^/help$'), self.on_help),
            (re.compile(r'^/players$'), self.on_players),
            (re.compile(r'^/give\s+(-?\S+)\s*,?\s*(-?[0-9]+)\s*,?\s*(-?[0-9]+)$'), self.on_give),
            (re.compile(r'^/dumpinv$'), self.dump_inventory),
        ]
    def start(self):
        thread = threading.Thread(target=self.run)
        thread.setDaemon(True)
        thread.start()
    def run(self):
        self.connection = sqlite3.connect(DB_PATH)
        self.create_tables()
        self.commit()
        while True:
            try:
                if time.time() - self.last_commit > COMMIT_INTERVAL:
                    self.commit()
                self.dequeue()
            except Exception:
                traceback.print_exc()
    def enqueue(self, func, *args, **kwargs):
        self.queue.put((func, args, kwargs))
    def dequeue(self):
        try:
            func, args, kwargs = self.queue.get(timeout=5)
            func(*args, **kwargs)
        except Queue.Empty:
            pass
    def execute(self, *args, **kwargs):
        return self.connection.execute(*args, **kwargs)
    def commit(self):
        self.last_commit = time.time()
        self.connection.commit()
    def create_tables(self):
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
        for query in queries:
            self.execute(query)
    def next_client_id(self):
        result = 1
        client_ids = set(x.client_id for x in self.clients)
        while result in client_ids:
            result += 1
        return result
    def on_connect(self, client):
        client.client_id = self.next_client_id()
        client.nick = 'player%d' % client.client_id
        client.inventory = []
        for slot in xrange(INVENTORY_SLOTS):
            client.inventory.append({"type" : 0, "count" : 0})
        log('CONN', client.client_id, *client.client_address)
        client.position = SPAWN_POINT
        self.clients.append(client)
        client.send(YOU, client.client_id, *client.position)
        client.send(TALK, 'Welcome to Craft!')
        client.send(TALK, 'Type "/help" for chat commands.')
        self.send_position(client)
        self.send_positions(client)
        self.send_talk('%s has joined the game.' % client.nick)
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
        self.send_talk('%s has disconnected from the server.' % client.nick)
    def on_chunk(self, client, p, q, key=0):
        p, q, key = map(int, (p, q, key))
        query = (
            'select rowid, x, y, z, w from block where '
            'p = :p and q = :q and rowid > :key;'
        )
        rows = self.execute(query, dict(p=p, q=q, key=key))
        max_rowid = 0
        for rowid, x, y, z, w in rows:
            client.send(BLOCK, p, q, x, y, z, w)
            max_rowid = max(max_rowid, rowid)
        if max_rowid:
            client.send(KEY, p, q, max_rowid)
    def on_block(self, client, x, y, z, w, s=0):
        x, y, z, w, s = map(int, (x, y, z, w, s))
        if y <= 0 or y > 255 or w < 0 or w > 11:
            return
        if (w == 0):
            # Break
            final_slot = -1
            for slot in xrange(INVENTORY_SLOTS):
                if client.inventory[slot]["type"] == w:
                    if client.inventory[slot]["count"] < 64:
                        final_slot = slot
                        break
            if (final_slot == -1):
                for slot in xrange(INVENTORY_SLOTS):
                    if client.inventory[slot]["type"] == 0:
                        final_slot = slot
                        client.inventory[slot]["count"] = 0
            
            client.inventory[final_slot]["type"] = w
            client.inventory[final_slot]["count"] += 1

        else:
            # Place
            success = False
            if (client.inventory[s]["type"] == w):
                client.inventory[s]["count"] -= 1
                if (client.inventory[slot]["count"] == 0):
                    client.inventory[slot]["type"] = 0
                client.send(INVENTORY, s, client.inventory[s]["type"], client.inventory[s]["count"])
            else:
                for slot in xrange(INVENTORY_SLOTS):
                    if client.inventory[slot]["type"] == w:
                        client.inventory[slot]["count"] -= 1
                        if (client.inventory[slot]["count"] == 0):
                            client.inventory[slot]["type"] = 0
                        client.send(INVENTORY, slot, client.inventory[slot]["type"], client.inventory[slot]["count"])

        p, q = chunked(x), chunked(z)
        query = (
            'insert or replace into block (p, q, x, y, z, w) '
            'values (:p, :q, :x, :y, :z, :w);'
        )
        self.execute(query, dict(p=p, q=q, x=x, y=y, z=z, w=w))
        self.send_block(client, p, q, x, y, z, w)
        if chunked(x - 1) != p:
            self.execute(query, dict(p=p - 1, q=q, x=x, y=y, z=z, w=-w))
            self.send_block(client, p - 1, q, x, y, z, -w)
        if chunked(x + 1) != p:
            self.execute(query, dict(p=p + 1, q=q, x=x, y=y, z=z, w=-w))
            self.send_block(client, p + 1, q, x, y, z, -w)
        if chunked(z - 1) != q:
            self.execute(query, dict(p=p, q=q - 1, x=x, y=y, z=z, w=-w))
            self.send_block(client, p, q - 1, x, y, z, -w)
        if chunked(z + 1) != q:
            self.execute(query, dict(p=p, q=q + 1, x=x, y=y, z=z, w=-w))
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
                    break
            else:
                client.send(TALK, 'Unrecognized command: "%s"' % text)
        else:
            self.send_talk('%s> %s' % (client.nick, text))
    def on_nick(self, client, nick=None):
        if nick is None:
            client.send(TALK, 'Your nickname is %s' % client.nick)
        else:
            self.send_talk('%s is now known as %s' % (client.nick, nick))
            client.nick = nick
    def on_spawn(self, client):
        client.position = SPAWN_POINT
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
    def on_pq(self, client, p, q):
        p, q = map(int, (p, q))
        if abs(p) > 1000 or abs(q) > 1000:
            return
        client.position = (p * CHUNK_SIZE, 0, q * CHUNK_SIZE, 0, 0)
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
    def on_inventory(self, client, w, slot, count):
        client.inventory[slot]["type"] = w
        client.inventory[slot]["count"] = count
    def on_give(self, client, nick=None, w=0, count=0):
        w, count = map(int, (w, count))
        if nick is None:
            other = client
        else:
            nicks = dict((client.nick, client) for client in self.clients)
            other = nicks.get(nick)
        print "Giving {0} {1} of {2}".format(nick, w, count)
        if w == 0 or count == 0:
            return
        for slot in xrange(INVENTORY_SLOTS):
            if other.inventory[slot]["type"] == w:
                if other.inventory[slot]["count"] + count > 64:
                    count -= 64 - abs(other.inventory[slot]["count"])
                    other.inventory[slot]["count"] = 64
                    other.send(INVENTORY, slot, other.inventory[slot]["type"], other.inventory[slot]["count"])
                else:
                    other.inventory[slot]["count"] += count
                    other.send(INVENTORY, slot, other.inventory[slot]["type"], other.inventory[slot]["count"])
                    return
        for slot in xrange(INVENTORY_SLOTS):
            if other.inventory[slot]["type"] == 0:
                if count > 64:
                    count -= 64
                    other.inventory[slot]["type"] = w
                    other.inventory[slot]["count"] = 64
                    other.send(INVENTORY, slot, other.inventory[slot]["type"], other.inventory[slot]["count"])
                else:
                    other.inventory[slot]["type"] = w
                    other.inventory[slot]["count"] = count
                    other.send(INVENTORY, slot, other.inventory[slot]["type"], other.inventory[slot]["count"])
                    return

    def dump_inventory(self, client):
        print client.inventory
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
    def send_talk(self, text):
        for client in self.clients:
            client.send(TALK, text)
    def send_inventory(self, client):
        for slot in xrange(INVENTORY_SLOTS):
            client.send(INVENTORY, slot, client.inventory[slot]["type"], client.inventory[slot]["count"])

def main():
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
