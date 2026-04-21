CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -g3
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
UUID_LIB = -luuid
else
UUID_LIB =
endif
LDFLAGS = $(UUID_LIB) -L./libs/myteams -lmyteams -Wl,-rpath,./libs/myteams
TEST_LDFLAGS = $(UUID_LIB)
CLIENT_TARGET = myteams_cli
TARGET = myteams_server
BIN             = unit_tests
COVFLAGS        = --coverage
GCOVR = .venv/bin/gcovr
SERVER_SOURCES = src/server/server.cpp src/server/protocol/Protocol.cpp src/server/CommandHandler/CommandHandler.cpp src/server/ServerState.cpp src/Entities.cpp
CLIENT_SOURCES = src/client/client.cpp src/client/ClientCommand.cpp src/client/ResponseHandler.cpp
SERVEROBJECTS = $(SERVER_SOURCES:.cpp=.o)
CLIENTOBJECTS = $(CLIENT_SOURCES:.cpp=.o)
TEST_SOURCES = \
	tests/test_main.cpp \
	tests/test_protocol.cpp \
	tests/test_server_state.cpp \
	tests/test_entities.cpp \
	tests/test_client_command.cpp \
	tests/test_response_handler.cpp \
	tests/test_command_handler.cpp \
	tests/stubs_logging.cpp
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)
TESTED_SOURCES = \
	src/client/ClientCommand.cpp \
	src/client/ResponseHandler.cpp \
	src/server/protocol/Protocol.cpp \
	src/server/ServerState.cpp \
	src/server/CommandHandler/CommandHandler.cpp \
	src/Entities.cpp
LIB_OBJECTS = $(TESTED_SOURCES:.cpp=.o)

all: $(TARGET) $(CLIENT_TARGET)

server: $(TARGET)

client: $(CLIENT_TARGET)

RED := \033[0;31m
GREEN := \033[0;32m
RESET := \033[0m

$(TARGET): $(SERVEROBJECTS)
	@echo "$(GREEN)[LINKING]$(RESET) $@"
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SERVEROBJECTS) $(LDFLAGS)
	mkdir -p obj/server
	mv $(SERVEROBJECTS) obj/server/

$(CLIENT_TARGET): $(CLIENTOBJECTS)
	@echo "$(GREEN)[LINKING]$(RESET) $@"
	$(CXX) $(CXXFLAGS) -o $(CLIENT_TARGET) $(CLIENTOBJECTS) $(LDFLAGS)
	mkdir -p obj/client
	mv $(CLIENTOBJECTS) obj/client/

%.o: %.cpp
	@echo "$(GREEN)[COMPILING]$(RESET) $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

tests/%.o: tests/%.cpp
	@echo "$(GREEN)[COMPILING TEST]$(RESET) $<"
	$(CXX) $(CXXFLAGS) $(COVFLAGS) -c $< -o $@

$(BIN): CXXFLAGS += $(COVFLAGS)

$(BIN): $(LIB_OBJECTS) $(TEST_OBJECTS)
	@echo "$(GREEN)[LINKING TESTS]$(RESET) $@"
	$(CXX) $(CXXFLAGS) $(COVFLAGS) -o $(BIN) $(LIB_OBJECTS) $(TEST_OBJECTS) $(TEST_LDFLAGS)

tests_run: clean $(BIN)
	./$(BIN) --verbose

coverage: tests_run
	$(GCOVR) --gcov-executable "llvm-cov gcov" \
		--filter "src/client/ClientCommand.cpp" \
		--filter "src/client/ResponseHandler.cpp" \
		--filter "src/server/protocol/Protocol.cpp" \
		--filter "src/server/ServerState.cpp" \
		--filter "src/server/CommandHandler/CommandHandler.cpp" \
		--filter "src/Entities.cpp" \
		--print-summary

clean:
	rm -f $(SERVEROBJECTS)
	rm -f $(CLIENTOBJECTS)
	rm -f $(TEST_OBJECTS)
	rm -f $(LIB_OBJECTS)
	rm -f *~
	rm -f src/*~
	rm -f include/*~
	rm -f tests/*~
	rm -f tests/*.o
	rm -f src/*.o
	rm -rf obj
	rm -f *.gcda *.gcno *.gcov
	rm -f src/*.gcda src/*.gcno
	rm -f src/client/*.gcda src/client/*.gcno
	rm -f src/server/*.gcda src/server/*.gcno
	rm -f src/server/protocol/*.gcda src/server/protocol/*.gcno
	rm -f src/server/CommandHandler/*.gcda src/server/CommandHandler/*.gcno
	rm -f tests/*.gcda tests/*.gcno

fclean: clean
	rm -f $(TARGET)
	rm -f $(CLIENT_TARGET)
	rm -f $(BIN)

re: fclean all

.PHONY: all clean fclean re
