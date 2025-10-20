NAME        = webserv
SRC_DIR     = src
INC_DIR     = include
SRC_FILES   = main.cpp \
              utils/utils.cpp
OBJ_DIR     = obj
OBJ_FILES   = $(addprefix $(OBJ_DIR)/,$(SRC_FILES:.cpp=.o))

CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98 -I$(INC_DIR)

all: $(NAME)

$(NAME): $(OBJ_FILES)
	@echo "Linking $(NAME)..."
	@$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ_FILES)
	@echo "✅ Build complete: ./$(NAME)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "Compiled: $<"

clean:
	@echo "🧹 Cleaning object files..."
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo "🧹 Removing binary..."
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re