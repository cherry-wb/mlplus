PROJECT_DIR = ..
GMOCK_DIR = ../external
# Where to find user code.

# Flags passed to the preprocessor.
CPPFLAGS +=  -I$(GMOCK_DIR)/include -I$(PROJECT_DIR)/include -I$(PROJECT_DIR)/src/ -L$(GMOCK_DIR)/lib -L.

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra 

# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
# All Google Test headers
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# All Google Mock headers
GMOCK_HEADERS = $(GMOCK_DIR)/include/gmock/*.h \
                $(GMOCK_DIR)/include/gmock/internal/*.h \
                $(GTEST_HEADERS)

# House-keeping build targets.

SRC = $(PROJECT_DIR)/src
INCLUDE = $(PROJECT_DIR)/include

SRCS=abstract_instance.cpp instance.cpp  attribute.cpp instance_container.cpp attribute_container.cpp special_functions.cpp estimator.cpp  normal_estimator.cpp discrete_estimator.cpp binary_estimator.cpp dataset.cpp expression.cpp scope.cpp lexer.cpp names_file_reader.cpp attribute_spec.cpp datetime.cpp naive_bayes.cpp decision_tree.cpp log.cpp
OBJ=$(SRCS:.cpp=.o)

all:$(OBJ) libmlplus_common.a names_reader_unittest string_utility_unittest attribute_unittest instance_unittest special_function_unittest estimator_unittest  naive_bayes_unittest expression_unittest datetime_unittest attribute_spec_unittest decision_tree_unittest text_parser_unittest gbdt_unittest

%.o: $(SRC)/%.cpp 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

libmlplus_common.a: $(OBJ) 
	$(AR) rcs $@ $^ 

text_parser_unittest: text_parser_unittest.cpp $(SRC)/io/text_parser.cpp libmlplus_common.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lgmock -lgmock_main -lpthread $^ -lmlplus_common -o $@

attribute_spec_unittest:attribute_spec_unittest.cpp libmlplus_common.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lgmock -lgmock_main -lpthread  $^ -lmlplus_common -o $@

decision_tree_unittest:decision_tree_unittest.cpp $(SRC)/io/text_parser.cpp libmlplus_common.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lgmock -lgmock_main -lpthread $^ -lmlplus_common -o $@
names_reader_unittest: names_file_reader_unittest.cpp libmlplus_common.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lgmock -lgmock_main -lpthread $^ -lmlplus_common -o $@

datetime_unittest: datetime_unittest.cpp libmlplus_common.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lgmock -lgmock_main -lpthread $^ -lmlplus_common -o $@

expression_unittest:  lexer_unittest.cpp libmlplus_common.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -lgmock -lgmock_main -lpthread -lmlplus_common -o $@

string_utility_unittest: string_trim_unittest.cpp string_convert_unittest.cpp  libmlplus_common.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lgmock -lgmock_main -lpthread $^ -o $@

attribute_unittest: attribute_unittest.cpp  libmlplus_common.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lgmock -lgmock_main -lpthread $^ -lmlplus_common -o $@

instance_unittest: instance_unittest.cpp  libmlplus_common.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lgmock -lgmock_main -lpthread $^ -lmlplus_common -o $@

special_function_unittest: special_function_unittest.cpp  libmlplus_common.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lgmock -lgmock_main -lpthread $^ -lmlplus_common -o $@

estimator_unittest: estimator_unittest.cpp  libmlplus_common.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lgmock -lgmock_main -lpthread $^ -lmlplus_common -o $@

naive_bayes_unittest: naive_bayes_unittest.cpp  libmlplus_common.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lgmock -lgmock_main -lpthread $^ -lmlplus_common -o $@

gbdt_unittest:gbdt_unittest.cpp 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lgmock -lgmock_main -lpthread $^ -o $@

clean :
	rm -f $(TESTS) *.o
	rm -f $(TESTS) *_unittest
	rm -f *.a


