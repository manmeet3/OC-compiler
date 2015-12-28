MKFILE    = Makefile
DEPSFILE  = ${MKFILE}.deps
NOINCLUDE = ci clean spotless
NEEDINCL  = ${filter ${NOINCLUDE}, ${MAKECMDGOALS}}
VALGRIND  = valgrind --leak-check=full --show-reachable=yes

HSOURCES  = astree.h  lyutils.h  auxlib.h  stringset.h symtable.h typecheck.h emit.h
CSOURCES  = astree.cpp lyutils.cpp auxlib.cpp stringset.cpp symtable.cpp typecheck.cpp emit.cpp oc.cpp
LSOURCES  = scanner.l
YSOURCES  = parser.y
ETCSRC    = README ${MKFILE} ${DEPSFILE}
CLGEN     = yylex.cpp
HYGEN     = yyparse.h
CYGEN     = yyparse.cpp
CGENS     = ${CLGEN} ${CYGEN}
ALLGENS   = ${HYGEN} ${CGENS}
EXECBIN   = oc
ALLCSRC   = ${CSOURCES} ${CGENS}
OBJECTS   = ${ALLCSRC:.cpp=.o}
LREPORT   = yylex.output
YREPORT   = yyparse.output
IREPORT   = ident.output
REPORTS   = ${LREPORT} ${YREPORT} ${IREPORT}
ALLSRC    = ${ETCSRC} ${YSOURCES} ${LSOURCES} ${HSOURCES} ${CSOURCES}

GCC       = g++ -g -O0 -Wall -Wextra -std=gnu++11
MKDEPS    = g++ -MM -std=gnu++11

all : ${EXECBIN}

${EXECBIN} : ${OBJECTS}
	${GCC} -o${EXECBIN} ${OBJECTS}
	ident ${OBJECTS} ${EXECBIN} >${IREPORT}

%.o : %.cpp
	${GCC} -c $<

${CLGEN} : ${LSOURCES}
	flex --outfile=${CLGEN} ${LSOURCES} 2>${LREPORT}
	- grep -v '^  ' ${LREPORT}

${CYGEN} ${HYGEN} : ${YSOURCES}
	bison --defines=${HYGEN} --output=${CYGEN} ${YSOURCES}

clean :
	- rm ${OBJECTS} ${ALLGENS} ${REPORTS} ${DEPSFILE}

spotless : clean
	- rm ${EXECBIN}

deps : ${ALLCSRC}
	@ echo "# ${DEPSFILE} created `date` by ${MAKE}" >${DEPSFILE}
	${MKDEPS} ${ALLCSRC} >>${DEPSFILE}

${DEPSFILE} :
	@ touch ${DEPSFILE}
	${MAKE} --no-print-directory deps


submit :
	submit cmps104a-wm.f15 asg5 ${ALLSRC}

again :
	gmake --no-print-directory spotless deps ci all lis
	
ifeq "${NEEDINCL}" ""
include ${DEPSFILE}
endif

