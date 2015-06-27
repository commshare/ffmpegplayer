objs = queue.o player.o main.o

main:$(objs)
	g++ -o1 -o main $(objs) $(CPPFLAGS) $(LDFLAGS) -lavformat -lavcodec -lavutil -ldl -lm -lpthread

clean:
	rm main $(objs)
