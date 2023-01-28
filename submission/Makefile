SCHEDULE = schedule
SCHEDULE_SRCS = schedule.c
SCHEDULE_HDRS = schedule.h
SCHEDULE_OBJS = $(SCHEDULE_SRCS:%.c=%.o)


CFLAGS = -std=gnu99 -g

default: $(SCHEDULE)

$(SCHEDULE_OBJS): $(SCHEDULE_HDRS)

$(SCHEDULE): $(SCHEDULE_OBJS)
	$(CC) $(CFLAGS) -o $@ $(SCHEDULE_OBJS)

test: $(SCHEDULE)
	./$(SCHEDULE) 1

clean:
	rm -f $(SCHEDULE_OBJS)
	rm -f $(SCHEDULE)
