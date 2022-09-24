all: libproteus tests

libproteus: libproteus.so
tests: proteus_tests libproteus


LIB_OBJS = \
	lib/Celestial.o \
	lib/Compass.o \
	lib/Decompress.o \
	lib/ErrLog.o \
	lib/GeoInfo.o \
	lib/GeoPos.o \
	lib/GeoVec.o \
	lib/Ocean.o \
	lib/ScalarConv.o \
	lib/Wave.o \
	lib/Weather.o \
	lib/proteus.o

TESTS_OBJS = \
	tests/tests_main.o \
	tests/test_Celestial.o \
	tests/test_Compass.o \
	tests/test_GeoInfo.o \
	tests/test_GeoPos.o \
	tests/test_GeoVec.o \
	tests/test_Ocean.o \
	tests/test_ScalarConv.o \
	tests/test_Wave.o \
	tests/test_Weather.o


lib/%.o: lib/%.c
	$(CC) -fvisibility=hidden -fPIC -c -Wall -Wextra -Iinclude -O2 -D_GNU_SOURCE -o $@ $<

libproteus.so: $(LIB_OBJS)
	$(CC) -fvisibility=hidden -shared -fPIC -O2 -o libproteus.so lib/*.o -lm -lz -lpthread


tests/%.o: tests/%.c
	$(CC) -fPIC -c -Wall -Wextra -Iinclude -O2 -D_GNU_SOURCE -o $@ $<

proteus_tests: $(TESTS_OBJS) libproteus.so
	$(CC) -O2 -o proteus_tests tests/*.o -L. -lproteus


clean:
	rm -rf lib/*.o tests/*.o libproteus.so proteus_tests
