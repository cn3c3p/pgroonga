CREATE TABLE memos (
  id integer,
  content varchar(256)
);

INSERT INTO memos VALUES (1, 'PostgreSQL is a RDBMS.');
INSERT INTO memos VALUES (2, 'Groonga is fast full text search engine.');
INSERT INTO memos VALUES (3, 'PGroonga is a PostgreSQL extension that uses Groonga.');

CREATE INDEX pgrn_index ON memos
 USING pgroonga (content pgroonga_varchar_full_text_search_ops_v2);

SET enable_seqscan = off;
SET enable_indexscan = off;
SET enable_bitmapscan = on;

EXPLAIN (COSTS OFF)
SELECT id, content, pgroonga_score(tableoid, ctid)
  FROM memos
 WHERE content &@ ('Groonga', ARRAY[0], 'pgrn_index')::pgroonga_full_text_search_condition;

SELECT id, content, pgroonga_score(tableoid, ctid)
  FROM memos
 WHERE content &@ ('Groonga', ARRAY[0], 'pgrn_index')::pgroonga_full_text_search_condition;

DROP TABLE memos;
