SRCS =						\
	src/pgroonga.c				\
	src/pgrn_column_name.c			\
	src/pgrn_command_escape_value.c		\
	src/pgrn_convert.c			\
	src/pgrn_create.c			\
	src/pgrn_ctid.c				\
	src/pgrn_escape.c			\
	src/pgrn_flush.c			\
	src/pgrn_global.c			\
	src/pgrn_groonga.c			\
	src/pgrn_groonga_tuple_is_alive.c	\
	src/pgrn_highlight_html.c		\
	src/pgrn_index_status.c			\
	src/pgrn_jsonb.c			\
	src/pgrn_keywords.c			\
	src/pgrn_match_positions_byte.c		\
	src/pgrn_match_positions_character.c	\
	src/pgrn_options.c			\
	src/pgrn_pg.c				\
	src/pgrn_query_escape.c			\
	src/pgrn_query_extract_keywords.c	\
	src/pgrn_snippet_html.c			\
	src/pgrn_value.c			\
	src/pgrn_variables.c			\
	src/pgrn_wal.c				\
	vendor/xxHash/xxhash.c