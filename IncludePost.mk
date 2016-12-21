all: subdirs $(LIB_TARGET) $(BIN_TARGET)

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

install: $(BIN_TARGET)
ifneq ($(BIN_TARGET),)
	@install -d $(EXEC_PREFIX)
	@install -t $(EXEC_PREFIX) $(BIN_TARGET)
endif
	@for dir in $(SUBDIR); do\
		make -C $$dir install TOPDIR=$(TOPDIR);\
	done


dev_install: 
ifneq ($(LIB_TARGET),)
	@install -d $(INC_PREFIX)
	@install -t $(INC_PREFIX) *.h
	@install -d $(LIB_PREFIX)
	@install -t $(LIB_PREFIX) $(LIB_TARGET)
endif
	@for dir in $(SUBDIR); do\
		make -C $$dir dev_install TOPDIR=$(TOPDIR);\
	done


clean:
	@rm -f *.so *.a $(LIB_OBJS) $(BIN_OBJS) *.d
	@for dir in $(SUBDIR); do\
		make -C $$dir clean;\
	done
	
