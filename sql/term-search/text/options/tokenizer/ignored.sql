CREATE TABLE tags (
  name text
);

CREATE INDEX pgrn_index ON tags
  USING pgroonga (name pgroonga.text_term_search_ops_v2)
  WITH (tokenizer = "TokenDelimit");

SELECT pgroonga.command('table_list')::json#>>'{1,2,6}';