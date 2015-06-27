objs = queue.o player.o main.o

main:$(objs)
	g++ -o1 -o main $(objs) $(CPPFLAGS) $(LDFLAGS) -lSDL -lavformat -lavcodec -lavutil -lswscale -ldl -lm -lpthread

clean:
	rm main $(objs)
