NAME		= webserv
CXX			= c++
CXXFLAGS	= -Wall -Werror -Wextra -std=c++98

SRCS		= config/Server.cpp \
					config/Location.cpp \
					utils/StatusException.cpp \
					core/Kqueue.cpp \
					core/Event.cpp \
					core/Socket.cpp \
					core/Connection.cpp \
					core/ServerManager.cpp \
					core/EventLoop.cpp \
					main.cpp

OBJS		= $(SRCS:%.cpp=%.o)

all	:		$(NAME)

$(NAME) : 	$(OBJS)
			$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

%.o	: 		%.cpp
			$(CXX) $(CXXFLAGS) -c $^ -I./ -o $@

clean	:
			rm -f $(OBJS)

fclean	:
			make clean
			rm -f $(NAME)

re	:
			make fclean
			make all

.PHONY	:	all clean fclean re