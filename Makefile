OS = $(shell uname -s | tr '[[:upper:]]' '[[:lower:]]')

ifeq ($(OS), darwin)
	CXX = xcrun -sdk macosx clang++
else
	CXX = clang++-3.5
endif

LD = $(CXX)

GSL_CFLAGS = $(shell pkg-config gsl       --cflags)
GTK_CFLAGS = $(shell pkg-config gtkmm-3.0 --cflags)

GSL_LIBS = $(shell pkg-config gsl       --libs)
GTK_LIBS = $(shell pkg-config gtkmm-3.0 --libs)


CXFLAGS = -c \
          -std=c++14 \
          -Wall \
          -O3 \
          -flto \
          -I /usr/local/include \
          -UNDEBUG \
          $(GSL_CFLAGS) $(GTK_CFLAGS)

LDFLAGS = -O3 \
          -flto \
          -Wl,-w \
          $(GSL_LIBS) $(GTK_LIBS)

OBJECTS = main.o hr_function.o


all: hr

hr: $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)

%.o: %.cc
	$(CXX) $(CXFLAGS) -o $@ $<

clean:
	rm -f $(OBJECTS) hr

.PHONY: all clean
