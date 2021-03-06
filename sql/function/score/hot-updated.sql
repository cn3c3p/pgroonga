CREATE TABLE memos (
  id integer PRIMARY KEY,
  tag varchar(256),
  content text
);

CREATE INDEX grnindex ON memos USING pgroonga (id, content);

INSERT INTO memos VALUES (1, 'pgsql', 'PostgreSQL is a RDBMS.');
INSERT INTO memos VALUES (2, 'groonga', 'Groonga is fast full text search engine.');
INSERT INTO memos VALUES (3, 'pgsql', 'PGroonga is a PostgreSQL extension that uses Groonga.');
UPDATE memos SET tag = 'groonga'
 WHERE id = 3;

SET enable_seqscan = off;
SET enable_indexscan = on;
SET enable_bitmapscan = off;

SELECT id, content, pgroonga_score(memos)
  FROM memos
 WHERE content @@ 'PGroonga OR Groonga';

DROP TABLE memos;
