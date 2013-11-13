PROJECT_DIR = ..
#-DTREE_DEBUG
CPPFLAGS += -I$(GMOCK_DIR)/include -I$(PROJECT_DIR)/include -L$(GMOCK_DIR)/lib -L.
# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -O2

DIR = $(PROJECT_DIR)/src
INCLUDE = $(PROJECT_DIR)/include
SRCS = bayes_message_passing.cpp  naive_bayes.cpp abstract_instance.cpp instance.cpp  attribute.cpp instance_container.cpp attribute_container.cpp special_functions.cpp estimator.cpp  normal_estimator.cpp discrete_estimator.cpp binary_estimator.cpp dataset.cpp expression.cpp scope.cpp lexer.cpp names_file_reader.cpp attribute_spec.cpp datetime.cpp naive_bayes.cpp decision_tree.cpp log.cpp
OBJ=$(SRCS:.cpp=.o)

all : bayes_msg_passing naive_bayes_train naive_bayes_classify naive_bayes_train_sparse naive_bayes_classify_sparse decision_tree_classifier libnaive_bayes_core.a $(OBJ)

%.o: $(DIR)/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

libnaive_bayes_core.a: $(OBJ) 
	$(AR) rcs $@ $^ 

naive_bayes_train: naive_bayes_train.cpp libnaive_bayes_core.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -lnaive_bayes_core -lm -lpthread -o $@

naive_bayes_classify: naive_bayes_classify.cpp  libnaive_bayes_core.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -lnaive_bayes_core -lm -lpthread -o $@ 

bayes_msg_passing: bayes_msg_passing.cpp libnaive_bayes_core.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -lnaive_bayes_core -lm -lpthread -lboost_program_options -o $@ 

naive_bayes_train_sparse:  naive_bayes_train_sparse.cpp  libnaive_bayes_core.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -lnaive_bayes_core -lm -lpthread -o $@

naive_bayes_classify_sparse:  naive_bayes_classify_sparse.cpp  libnaive_bayes_core.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -lnaive_bayes_core -lm -lpthread -o $@
decision_tree_classifier:decision_tree_classifier.cpp libnaive_bayes_core.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -lnaive_bayes_core -lm -lpthread -o $@
decision_tree_classifier_debug :decision_tree_classifier.cpp libnaive_bayes_core.a
	$(CXX) -DTREE_DEBUG $(CPPFLAGS) $(CXXFLAGS) $< -lnaive_bayes_core -lm -lpthread -o $@
clean :
	rm -f $(TESTS) *.o
	rm -f *.a
	rm naive_bayes_train 
	rm naive_bayes_classify
	rm naive_bayes_train_sparse
	rm naive_bayes_classify_sparse

