TARGET = ircserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror

SRCDIR = src
OBJDIR = obj

INCLUDE = include

SOURCES = $(shell find $(SRCDIR) -type f -name '*.cpp')

OBJS = $(shell find $(SRCDIR) -type f -name '*.cpp' | sed s/^$(SRCDIR)/$(OBJDIR)/)
OBJS := $(OBJS:.cpp=.o)

DEPS = $(OBJS:.o=.d)

.PHONY: all re clean fclean

all: $(TARGET)

re:
	$(MAKE) fclean
	$(MAKE) all

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -rf $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@

$(OBJDIR):
	mkdir -p `find $(SRCDIR) -type d | sed s/^$(SRCDIR)/$(OBJDIR)/`

obj/%.o: src/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(addprefix -I,$(INCLUDE)) -MMD -c $< -o $@

-include $(DEPS)