@echo off
clang++ -O2 -o server .\Server\DatabaseMain.cpp .\Server\DatabaseUtils.cpp .\Server\DatabaseNet.cpp -lsqlite3

javac .\Client\*.java
jar cfe Requestor.jar Requestor .\Client\Requestor.class
jar uf Requestor.jar .\Client\*.class