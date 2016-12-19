all: subdirs $(BIN_TARGET) $(LIB_TARGET)

$(BIN_TARGET): $(BIN_OBJS)
	$(CC) -o $@ $^ $(BIN_LDFLAGS)

$(LIB_TARGET) : $(LIB_OBJS)
	$(AR) rcs $@ $^

$(BIN_OBJS) : %.o : %.c 
	$(CC) $(BIN_CFLAGS) -o $@ -c $<

$(LIB_OBJS) : %.o : %.c 
	$(CC) $(LIB_CFLAGS) -o $@ -c $<

%.d : %.c 
	@$(CC) -MM $(CFLAGS) $< > $@

-include $(patsubst %.c,%.d,$(wildcard *.c))

subdirs:
	@for dir in $(SUBDIR); do\
		make -C $$dir TOPDIR=$(TOPDIR);\
	done
	
clean:
	rm -f *.so *.a $(LIB_OBJS) $(BIN_OBJS) *.d
	@for dir in $(SUBDIR); do\
		make -C $$dir clean;\
	done
	
