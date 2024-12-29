CC = gcc
CFLAGS = -Wall -Wextra -O2
INCLUDES = -I include -I lib/RIO/include
AR = ar
RM = rm -f
BUILD_DIR = build

RIO_DIR = lib/RIO
RIO_STATIC_LIB = $(RIO_DIR)/librio.a

all: $(RIO_STATIC_LIB) $(BUILD_DIR) \
	 $(BUILD_DIR)/libnetwork.a \
	 $(BUILD_DIR)/libserver.a \
	 $(BUILD_DIR)/server \
	 $(BUILD_DIR)/client

# 1. Build the RIO library (produces librio.a and/or librio.so)
$(RIO_STATIC_LIB):
	$(MAKE) -C $(RIO_DIR)

# 2. Create build directory if not present
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 3. Object files
$(BUILD_DIR)/network.o: src/network.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/server.o: src/server.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

$(BUILD_DIR)/client.o: src/client.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/main.o: main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 4. Network library
$(BUILD_DIR)/libnetwork.a: $(BUILD_DIR)/network.o
	$(AR) rcs $@ $^

# 5. Server library (depends on server.o, which in turn uses network headers)
$(BUILD_DIR)/libserver.a: $(BUILD_DIR)/server.o
	$(AR) rcs $@ $^

# 6. Link main into the final "server" executable 
#    (depends on server library, network library, and RIO library)
$(BUILD_DIR)/server: $(BUILD_DIR)/main.o \
					 $(BUILD_DIR)/libserver.a \
					 $(BUILD_DIR)/libnetwork.a \
					 $(RIO_STATIC_LIB)
	$(CC) $(CFLAGS) -o $@ \
		$(BUILD_DIR)/main.o \
		$(BUILD_DIR)/libserver.a \
		$(BUILD_DIR)/libnetwork.a \
		$(RIO_STATIC_LIB)

# 7. Link client into the final “client” executable 
#    (depends on network library; no need for server library)
$(BUILD_DIR)/client: $(BUILD_DIR)/client.o \
					 $(BUILD_DIR)/libnetwork.a \
					 $(RIO_STATIC_LIB)
	$(CC) $(CFLAGS) -o $@ \
		$(BUILD_DIR)/client.o \
		$(BUILD_DIR)/libnetwork.a \
		$(RIO_STATIC_LIB)

clean:
	$(RM) $(BUILD_DIR)/*.o $(BUILD_DIR)/*.a \
		  $(BUILD_DIR)/server $(BUILD_DIR)/client
	rmdir $(BUILD_DIR) 2>/dev/null || true
	$(MAKE) -C $(RIO_DIR) clean

.PHONY: all clean