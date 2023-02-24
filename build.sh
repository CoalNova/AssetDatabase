clang++ -O2 -o server ./Server/DatabaseMain.cpp ./Server/DatabaseUtils.cpp ./Server/DatabaseNet.cpp -lsqlite3

cd Client
javac *.java
jar cfe Requestor.jar Requestor Requestor.class
jar uf Requestor.jar *.class
mv Requestor.jar ../Requestor.jar
cd ..