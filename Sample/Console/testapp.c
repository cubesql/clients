/*
 *  main.c
 *  testapp
 *
 *  Created by Marco Bambini on 01/23/11.
 *  Copyright 2011 SQLabs. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cubesql.h"

#define HOSTNAME	"localhost"
#define USERNAME	"admin"
#define PASSWORD	"admin"

void do_test (csqldb *db);
void print_cursor(csqlc *c);

void do_test (csqldb *db) {
	
	int err = 0;
	csqlc	*c = NULL;
	
	// create db
	err = cubesql_execute(db, "CREATE DATABASE mytestdb.sqlite IF NOT EXISTS;");
	if (err != CUBESQL_NOERR) goto abort;
	
	// set current db
	err = cubesql_execute(db, "USE DATABASE mytestdb.sqlite;");
	if (err != CUBESQL_NOERR) goto abort;
	
	// create table
	err = cubesql_execute(db, "CREATE TABLE IF NOT EXISTS foo (id INTEGER PRIMARY KEY AUTOINCREMENT, col1 TEXT, col2 TEXT, col3 INTEGER);");
	if (err != CUBESQL_NOERR) goto abort;
	
	// insert a couple records
	err = cubesql_execute(db, "INSERT INTO foo (col1, col2, col3) VALUES ('test1', 'test2', 13);");
	err = cubesql_execute(db, "INSERT INTO foo (col1, col2, col3) VALUES ('test3', 'test4', 17);");
	err = cubesql_execute(db, "INSERT INTO foo (col1, col2, col3) VALUES ('test5', 'test6', 19);");
	err = cubesql_execute(db, "INSERT INTO foo (col1, col2, col3) VALUES ('test7', 'test8', 23);");
	
	// commit current transaction
	err = cubesql_commit(db);

	// perform a simple select statement
	c = cubesql_select(db, "SELECT * FROM foo;", kFALSE);
	if (c == NULL) goto abort;
	print_cursor(c);
	cubesql_cursor_free(c);
	
	return;
	
abort:
	printf("An error occured in do_test: %s (errocode %d)", cubesql_errmsg(db), cubesql_errcode(db));
	return;
}

void print_cursor(csqlc *c)
{
	int		i, nrows, ncols, len;
	char	*s, b[512];
	
	if (c == NULL) return;
	
	nrows = cubesql_cursor_numrows(c);
	ncols = cubesql_cursor_numcolumns(c);
	printf("Record set contains\nrows: %d\ncolumns: %d\n\n", nrows, ncols);
	
	// print column's names
	for (i=1; i<=ncols; i++) {
		s = cubesql_cursor_field(c, CUBESQL_COLNAME, i, &len);
		printf("%s\t\t", s);
	}
	
	// print a separator
	printf("\n");
	for (i=1; i<=70; i++) printf("-");
	printf("\n");
	
	// print data using the EOF property (safe for both server side and client side cursors)
	while (cubesql_cursor_iseof(c) != kTRUE) {
		for (i=1; i<=ncols; i++) {
			s = cubesql_cursor_cstring_static(c, CUBESQL_CURROW, i, b, sizeof(b));
			printf("%s\t\t", s);
		}
		
		cubesql_cursor_seek(c, CUBESQL_SEEKNEXT);
		printf("\n");
	}
    printf("\n");
}

int main (void) {
    csqldb	*db = NULL;
	
	// connection without encryption
	if (cubesql_connect(&db, HOSTNAME, CUBESQL_DEFAULT_PORT, USERNAME, PASSWORD, CUBESQL_DEFAULT_TIMEOUT, CUBESQL_ENCRYPTION_AES256) != CUBESQL_NOERR)
		goto abort;
	
	// do a simple test
	do_test(db);
	
	// disconnect
	cubesql_disconnect(db, kTRUE);
	return 0;
	
abort:
	if (db) {
		printf("error %d in cubesql_connect: %s\n", cubesql_errcode(db), cubesql_errmsg(db));
		cubesql_disconnect(db, kFALSE);
	}
	return -1;
}
