#!/bin/bash
echo "Next two lines should be SUCCESS"
echo SUCCESS
echo "SUCCESS"
echo "\nSUCCESS indented three spaces"
echo "   SUCCESS"

echo "\nNext four lines should be SUCCESS"
echo SUCCESS | grep SUCCESS
echo SUCCESS | grep SUCCESS | grep -v FAILURE | grep SUCCESS
echo SUCCESS|grep SUCCESS
echo SUCCESS|grep SUCCESS|grep -v FAILURE|grep SUCCESS

echo "\nhello after 5 seconds each"
echo hello | cat | ./sleepycat 5
echo hello | ./sleepycat 5 | cat
echo hello|cat|./sleepycat 5
echo hello|./sleepycat 5|cat

echo "\nShould print out the \"rm -f ...\" line from Makefile"
cat Makefile | grep "f *"
grep "f *" < Makefile
grep "f *"<Makefile
grep "f *" Makefile

echo "\nShould print 2 lines."
grep "all " ../Makefile
echo "\nShould have \"cat < Makefile\""
echo "cat < Makefile" > foo.txt
cat foo.txt
echo "\nShould have \"cat < Makefile | grep all\""
echo "cat < Makefile | grep all" > foo.txt
cat foo.txt

echo "\nError Testing - should print Makefile"
cat Makefile
echo "\nError Testing - Should also print Makefile"
echo cat Makefile > test.sh
chmod a+x test.sh
./test.sh

echo "\nShould print Makefile"
cat < Makefile

echo "\nShould print 4 errors"
blablabla
cat < blablabla
mkdir tmp
cat Makefile > tmp
cat Makefile | blablabla
rmdir tmp


echo "\nShould print 3 lines"
echo "SUCCESS" > foo.txt
echo "huh?" >> foo.txt
echo bar abc def >> foo.txt
cat foo.txt
