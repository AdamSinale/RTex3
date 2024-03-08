
CC = gcc
FLAGS = -Wall -g

AR = ar
OBJECTS_SENDER = TCP_Sender.o
OBJECTS_RECEIVER = TCP_Receiver.o

all: TCP_Receiver TCP_Sender

	
TCP_Sender: $(OBJECTS_SENDER)
	$(CC) $(FLAGS) -o $@ $^
TCP_Receiver: $(OBJECTS_RECEIVER)
	$(CC) $(FLAGS) -o $@ $^

TCP_Sender.o: TCP_Sender.c
	$(CC) $(FLAGS) -c $<
TCP_Receiver.o: TCP_Receiver.c
	$(CC) $(FLAGS) -c $<

.PHONY: clean
clean:
	rm -f *.o TCP_Receiver TCP_Sender
