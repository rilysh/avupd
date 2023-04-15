FLAGS = -Wall -Wextra -O3 -s
LINK = -lX11
PROGRAM = avupd

all:
	${CC} ${FLAGS} ${PROGRAM}.c ${LINK} -o ${PROGRAM}

clean:
	rm -f ${PROGRAM}
