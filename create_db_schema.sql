create SEQUENCE block_rowid start 1;
create SEQUENCE light_rowid start 1;
create SEQUENCE sign_rowid start 1;
create SEQUENCE block_history_rowid start 1;

create table block (
  rowid bigint default nextval('public.block_rowid'::regclass) primary key,
  updated_at timestamp,
  user_id varchar(64) not null,
  p int not null,
  q int not null,
  x int not null,
  y int not null,
  z int not null,
  w int not null
);
alter table block add constraint unique_block_pqxyz unique (p,q,x,y,z);

create table if not exists light (
  rowid bigint default nextval('public.light_rowid'::regclass) primary key,
  p int not null,
  q int not null,
  x int not null,
  y int not null,
  z int not null,
  w int not null
);
create unique index if not exists light_pqxyz_idx on light (p, q, x, y, z);

create table if not exists sign (
  rowid bigint default nextval('public.sign_rowid'::regclass) primary key,
  p int not null,
  q int not null,
  x int not null,
  y int not null,
  z int not null,
  face int not null,
  text text not null
);
create index if not exists sign_pq_idx on sign (p, q);

create unique index if not exists sign_xyzface_idx on sign (x, y, z, face);
            
create table if not exists block_history (
  rowid bigint default nextval('public.block_history_rowid'::regclass),
  created_at timestamp,
  user_id varchar(64) not null,
  p int not null,
  q int not null,
  x int not null,
  y int not null,
  z int not null,
  w int not null,
  primary key (rowid,created_at)
) partition by range (created_at);

CREATE SCHEMA partman;
CREATE EXTENSION pg_partman WITH SCHEMA partman;

SELECT partman.create_parent( p_parent_table => 'public.block_history',
 p_control => 'created_at',
 p_type => 'native',
 p_interval=> 'monthly',
 p_premake => 24);
