#include "globals.h"
#include "code.h"

void emitComment (char* c) {
	if (c == NULL) return;
	if (TraceCode) {
		fprintf(code, "\n#%s\n", c);
	}
}

void emitCode (char* s) {
	if (s == NULL) return;
	fprintf(code, "%s\n", s);
}

void emitRO (char* op, int r, int s, int t) {
	if (op == NULL) return;
	fprintf(code, "%5s %d, %d, %d\n", op, r, s, t);
}
