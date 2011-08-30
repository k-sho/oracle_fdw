/*-------------------------------------------------------------------------
 *
 * oracle_fdw.h
 * 		This header file contains all definitions that are shared by
 * 		oracle_fdw.c and oracle_utils.c.
 * 		It is necessary to split oracle_fdw into two source files because
 * 		PostgreSQL and Oracle headers cannot be #included at the same time.
 *
 *-------------------------------------------------------------------------
 */

/* this one is safe to include and gives us Oid */
#include "postgres_ext.h"

#include <sys/types.h>

/* opaque type encapsulating the real Oracle connection */
typedef struct oracleSession oracleSession;

/* types for the Oracle table description */
typedef enum
{
	ORA_TYPE_VARCHAR2,
	ORA_TYPE_CHAR,
	ORA_TYPE_NVARCHAR2,
	ORA_TYPE_NCHAR,
	ORA_TYPE_NUMBER,
	ORA_TYPE_FLOAT,
	ORA_TYPE_BINARYFLOAT,
	ORA_TYPE_BINARYDOUBLE,
	ORA_TYPE_RAW,
	ORA_TYPE_DATE,
	ORA_TYPE_TIMESTAMP,
	ORA_TYPE_TIMESTAMPTZ,
	ORA_TYPE_INTERVALY2M,
	ORA_TYPE_INTERVALD2S,
	ORA_TYPE_BLOB,
	ORA_TYPE_CLOB,
	ORA_TYPE_BFILE,
	ORA_TYPE_OTHER
} oraType;

struct oraColumn
{
	char *name;              /* name in Oracle */
	oraType oratype;         /* data type in Oracle */
	int scale;               /* "scale" type modifier, used for NUMBERs */
	Oid pgtype;              /* PostgreSQL data type */
	int pgtypmod;            /* PostgreSQL type modifier */
	int used;                /* is the column used in the query? */
	char *val;               /* buffer for Oracle to return results in */
	long val_size;           /* allocated size in val */
	unsigned short val_len;  /* actual length of val */
	short val_null;          /* indicator for NULL value */
};

struct oraTable
{
	char *name;    /* name in Oracle */
	char *pgname;  /* for error messages */
	int ncols;     /* number of columns */
	int npgcols;   /* number of columns in the corresponding PostgreSQL foreign table */
	struct oraColumn **cols;
};

/* types to store parameter descriprions */

typedef enum {
	BIND_STRING,
	BIND_NUMBER,
	BIND_TIMESTAMP
} oraBindType;

struct paramDesc
{
	char *name;            /* name we give the parameter */
	int number;            /* index in the array of parameters */
	Oid type;              /* PostgreSQL data type */
	oraBindType bindType;  /* which type to use for binding to Oracle statement */
	char *value;           /* value rendered as string */
	int isExtern;          /* false for internal parameters */
	struct paramDesc *next;
};

/* PostgreSQL error messages we need */
typedef enum
{
	FDW_ERROR,
	FDW_UNABLE_TO_ESTABLISH_CONNECTION,
	FDW_UNABLE_TO_CREATE_REPLY,
	FDW_UNABLE_TO_CREATE_EXECUTION,
	FDW_TABLE_NOT_FOUND,
	FDW_OUT_OF_MEMORY
} oraError;

/*
 * functions defined in oracle_utils.c
 */
extern oracleSession *oracleGetSession(const char *connectstring, char *user, char *password, const char *nls_lang, const char *tablename);
extern void oracleReleaseSession(oracleSession *session, struct oraTable *oraTable, int close);
extern void oracleCloseStatement(oracleSession *session, const struct oraTable *oraTable);
extern void oracleCloseConnections(void);
extern void oracleShutdown(void);
extern int oracleIsStatementOpen(oracleSession *session);
extern struct oraTable *oracleDescribe(oracleSession *session, char *tablename, char *pgname);
extern void oracleEstimate(oracleSession *session, const char *query, double seq_page_cost, int block_size, double *startup_cost, double *total_cost, double *rows, int *width);
extern void oracleExplain(oracleSession *session, const char *query, int *nrows, char ***plan);
extern int oracleExecuteQuery(oracleSession *session, const char *query, const struct oraTable *oraTable, struct paramDesc *paramList);
extern int oracleFetchNext(oracleSession *session, struct oraTable *oraTable);
extern void oracleGetLob(oracleSession *session, struct oraTable *oraTable, void *locptr, char **value, long *value_len);

/*
 * functions defined in oracle_fdw.c
 */
extern void *oracleAlloc(size_t size);
extern void *oracleRealloc(void *p, size_t size);
extern void oracleFree(void *p);
extern void oracleError(oraError sqlstate, const char *message, const char *detail);
extern void oracleError_s(oraError sqlstate, const char *message, const char *arg, const char *detail);
extern void oracleError_i(oraError sqlstate, const char *message, int arg);