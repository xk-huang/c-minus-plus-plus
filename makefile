# encoding=utf-8
# author: xiaoke huang
# date: 2019/12/14


CC = g++-9
CFLAGS = 

compiler: main.o scanner.o parser.o analyzer.o symboltable.o \
 typecheck.o pcodegen.o
	$(CC) $(CFLAGS) $^ -o $@ -ll

main.o: main.cpp  scanner.h parser.h analyzer.h symboltable.h \
 typecheck.h pcodegen.h
	$(CC) $(CFLAGS) $< -c

parser.o: parser.cpp parser.h scanner.h 
	$(CC) $(CFLAGS) $< -c

scanner.o: scanner.cpp scanner.h 
	$(CC) $(CFLAGS) $< -c

symboltable.o: symboltable.cpp symboltable.h parser.h scanner.h \
 
	$(CC) $(CFLAGS) $< -c

typecheck.o: typecheck.cpp typecheck.h parser.h scanner.h \
 
	$(CC) $(CFLAGS) $< -c

analyzer.o: analyzer.cpp analyzer.h symboltable.h parser.h scanner.h \
  typecheck.h
	$(CC) $(CFLAGS) $< -c

pcodegen.o: pcodegen.cpp pcodegen.h analyzer.h \
 symboltable.h parser.h scanner.h  typecheck.h
	$(CC) $(CFLAGS) $< -c

.PHONY: clean
clean:
	rm ./*.o