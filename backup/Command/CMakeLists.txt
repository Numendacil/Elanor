
target_sources( ${ELANORBOT_TARGET_NAME} 
	PRIVATE 
		Command.hpp
		GroupCommandBase.hpp
		Answer.hpp
		Answer.cpp
		AtBot.hpp
		AtBot.cpp
		Bililive.hpp
		Bililive.cpp
		Recall.hpp
		Recall.cpp
		Repeat.hpp
		Repeat.cpp
		RollDice.hpp
		RollDice.cpp
		Sing.hpp
		Sing.cpp
	)

add_subdirectory(admin)
add_subdirectory(image)
add_subdirectory(pjsk)