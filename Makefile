CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -g3
LDFLAGS = -L./libs/myteams -lmyteams -Wl,-rpath,./libs/myteams
CLIENT_TARGET = myteams_client
TARGET = myteams_server
BIN             = unit_tests
CRITERION       = -lcriterion
COVFLAGS        = --coverage
SERVER_SOURCES = src/server/server.cpp src/server/protocol/Protocol.cpp src/server/CommandHandler/CommandHandler.cpp
CLIENT_SOURCES = src/client/client.cpp src/client/ClientCommand.cpp src/client/ResponseHandler.cpp
SERVEROBJECTS = $(SERVER_SOURCES:.cpp=.o)
CLIENTOBJECTS = $(CLIENT_SOURCES:.cpp=.o)

all: $(TARGET) $(CLIENT_TARGET)

server: $(TARGET)

client: $(CLIENT_TARGET)

RED := \033[0;31m
GREEN := \033[0;32m
RESET := \033[0m

$(TARGET): $(SERVEROBJECTS)
	@echo "$(GREEN)[LINKING]$(RESET) $@"
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SERVEROBJECTS)
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
	$(CXX) $(CXXFLAGS) $(COVFLAGS) -o $(BIN) $(LIB_OBJECTS) $(TEST_OBJECTS) $(CRITERION)

tests_run: clean $(BIN)
	./$(BIN) --verbose

coverage: tests_run
	gcovr --gcov-executable "llvm-cov gcov" --exclude tests/

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
	rm -f tests/*.gcda tests/*.gcno

fclean: clean
	rm -f $(TARGET)
	rm -f $(CLIENT_TARGET)
	rm -f $(BIN)

re: fclean all

.PHONY: all clean fclean re
