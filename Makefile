NAME		= webserv
CXX			= c++
CXXFLAGS	= -Wall -Werror -Wextra -std=c++98

SRCS		= config/Server.cpp \
			  config/Location.cpp \
			  utils/StatusException.cpp \
			  utils/Config.cpp \
				utils/Util.cpp \
			  core/Kqueue.cpp \
			  core/Event.cpp \
			  core/Socket.cpp \
			  server/Connection.cpp \
			  server/ServerManager.cpp \
			  server/EventLoop.cpp \
			  http/Request.cpp \
			  http/RequestParser.cpp \
			  http/Response.cpp \
			  http/AResponseBuilder.cpp \
			  http/ErrorBuilder.cpp \
				http/StaticFileBuilder.cpp \
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