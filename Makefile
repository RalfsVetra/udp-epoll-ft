TARGET_EXEC := udp_epoll_ft

BUILD_DIR := build
SRC_DIR   := src

CC       := gcc
CFLAGS   := -Wall -Wextra -O2
CPPFLAGS := -MMD -MP

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -r $(BUILD_DIR)

-include $(DEPS)

