#include "pgroonga.h"

#include "pgrn-alias.h"
#include "pgrn-column-name.h"
#include "pgrn-create.h"
#include "pgrn-global.h"
#include "pgrn-groonga.h"
#include "pgrn-options.h"
#include "pgrn-value.h"

static grn_ctx *ctx = &PGrnContext;
static struct PGrnBuffers *buffers = &PGrnBuffers;

static void
PGrnCreateSourcesCtidColumn(PGrnCreateData *data)
{
	data->sourcesCtidColumn = PGrnCreateColumn(data->index,
											   data->sourcesTable,
											   PGrnSourcesCtidColumnName,
											   GRN_OBJ_COLUMN_SCALAR,
											   grn_ctx_at(ctx, GRN_DB_UINT64));
}

void
PGrnCreateSourcesTable(PGrnCreateData *data)
{
	char buildingSourcesTableName[GRN_TABLE_MAX_KEY_SIZE];

	snprintf(buildingSourcesTableName, sizeof(buildingSourcesTableName),
			 PGrnBuildingSourcesTableNameFormat, data->relNode);
	if (PGrnIsAccessorAliasAvailable)
	{
		data->sourcesTable = PGrnCreateTable(data->index,
											 buildingSourcesTableName,
											 GRN_OBJ_TABLE_HASH_KEY,
											 grn_ctx_at(ctx, GRN_DB_UINT64),
											 NULL,
											 NULL,
											 NULL);
		data->sourcesCtidColumn = NULL;
	}
	else
	{
		data->sourcesTable = PGrnCreateTable(data->index,
											 buildingSourcesTableName,
											 GRN_OBJ_TABLE_NO_KEY,
											 NULL,
											 NULL,
											 NULL,
											 NULL);
		PGrnCreateSourcesCtidColumn(data);
	}
}

void
PGrnCreateSourcesTableFinish(PGrnCreateData *data)
{
	char sourcesTableName[GRN_TABLE_MAX_KEY_SIZE];

	snprintf(sourcesTableName, sizeof(sourcesTableName),
			 PGrnSourcesTableNameFormat, data->relNode);
	PGrnRenameTable(data->index, data->sourcesTable, sourcesTableName);
	if (!data->sourcesCtidColumn)
		PGrnAliasAdd(data->index);
}

void
PGrnCreateDataColumn(PGrnCreateData *data)
{
	grn_column_flags flags = 0;
	grn_obj *range;
	grn_id rangeID;
	bool compressable;

	if (data->forPrefixSearch)
	{
		char lexiconName[GRN_TABLE_MAX_KEY_SIZE];

		snprintf(lexiconName, sizeof(lexiconName),
				 PGrnLexiconNameFormat, data->relNode, data->i);
		range = PGrnLookup(lexiconName, ERROR);
		rangeID = grn_obj_id(ctx, range);
	}
	else
	{
		rangeID = data->attributeTypeID;
		range = grn_ctx_at(ctx, rangeID);
	}

	if (data->attributeFlags & GRN_OBJ_VECTOR)
	{
		flags |= GRN_OBJ_COLUMN_VECTOR;
		compressable = PGrnIsVectorCompressionAvailable;
	}
	else
	{
		flags |= GRN_OBJ_COLUMN_SCALAR;
		compressable = true;
	}

	if (compressable &&
		(PGrnIsLZ4Available || PGrnIsZlibAvailable || PGrnIsZstdAvailable))
	{
		switch (rangeID)
		{
		case GRN_DB_SHORT_TEXT:
		case GRN_DB_TEXT:
		case GRN_DB_LONG_TEXT:
			if (PGrnIsZstdAvailable)
			{
				flags |= GRN_OBJ_COMPRESS_ZSTD;
			}
			else if (PGrnIsLZ4Available)
			{
				flags |= GRN_OBJ_COMPRESS_LZ4;
			}
			else if (PGrnIsZlibAvailable)
			{
				flags |= GRN_OBJ_COMPRESS_ZLIB;
			}
			break;
		}
	}

	{
		char columnName[GRN_TABLE_MAX_KEY_SIZE];
		PGrnColumnNameEncode(data->desc->attrs[data->i]->attname.data,
							 columnName);
		PGrnCreateColumn(data->index,
						 data->sourcesTable,
						 columnName,
						 flags,
						 range);
	}
}

void
PGrnCreateLexicon(PGrnCreateData *data)
{
	grn_id typeID = GRN_ID_NIL;
	char lexiconName[GRN_TABLE_MAX_KEY_SIZE];
	grn_table_flags flags = GRN_OBJ_TABLE_PAT_KEY;
	grn_obj *type;
	grn_obj *lexicon;
	grn_obj *tokenizer = NULL;
	grn_obj *normalizer = NULL;
	grn_obj *tokenFilters = &(buffers->tokenFilters);

	GRN_BULK_REWIND(tokenFilters);
	switch (data->attributeTypeID)
	{
	case GRN_DB_TEXT:
	case GRN_DB_LONG_TEXT:
		typeID = GRN_DB_SHORT_TEXT;
		break;
	default:
		typeID = data->attributeTypeID;
		break;
	}

	if (data->forFullTextSearch ||
		data->forRegexpSearch ||
		data->forPrefixSearch)
	{
		const char *tokenizerName = NULL;
		const char *normalizerName = PGRN_DEFAULT_NORMALIZER;
		PGrnOptionUseCase useCase = PGRN_OPTION_USE_CASE_UNKNOWN;

		if (data->forFullTextSearch)
		{
			tokenizerName = PGRN_DEFAULT_TOKENIZER;
			useCase = PGRN_OPTION_USE_CASE_FULL_TEXT_SEARCH;
		}
		else if (data->forRegexpSearch)
		{
			tokenizerName = "TokenRegexp";
			useCase = PGRN_OPTION_USE_CASE_REGEXP_SEARCH;
		}
		else if (data->forPrefixSearch)
		{
			tokenizerName = NULL;
			useCase = PGRN_OPTION_USE_CASE_PREFIX_SEARCH;
		}

		PGrnApplyOptionValues(data->index,
							  useCase,
							  &tokenizer, tokenizerName,
							  &normalizer, normalizerName,
							  tokenFilters);
	}

	snprintf(lexiconName, sizeof(lexiconName),
			 PGrnLexiconNameFormat, data->relNode, data->i);
	type = grn_ctx_at(ctx, typeID);
	lexicon = PGrnCreateTable(data->index,
							  lexiconName,
							  flags,
							  type,
							  tokenizer,
							  normalizer,
							  tokenFilters);
	GRN_PTR_PUT(ctx, data->lexicons, lexicon);
}

void
PGrnCreateIndexColumn(PGrnCreateData *data)
{
	char lexiconName[GRN_TABLE_MAX_KEY_SIZE];
	grn_obj *lexicon;
	grn_column_flags flags = GRN_OBJ_COLUMN_INDEX;

	snprintf(lexiconName, sizeof(lexiconName),
			 PGrnLexiconNameFormat, data->relNode, data->i);
	lexicon = PGrnLookup(lexiconName, ERROR);

	if (data->forFullTextSearch || data->forRegexpSearch)
	{
		flags |= GRN_OBJ_WITH_POSITION;
		if (data->attributeFlags & GRN_OBJ_VECTOR)
		{
			flags |= GRN_OBJ_WITH_SECTION;
		}
	}
	PGrnCreateColumn(data->index,
					 lexicon,
					 PGrnIndexColumnName,
					 flags,
					 data->sourcesTable);
}
