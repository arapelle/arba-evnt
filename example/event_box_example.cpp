#include <evnt/evnt.hpp>
#include <iostream>

class int_event
{
public:
    int value;
};

int main()
{
    evnt::event_manager event_manager;
    evnt::event_box event_box;
    evnt::event_manager other_event_manager;

    int value = 0;
    other_event_manager.connect<int_event>([&value](int_event& event)
                                           {
                                               value = event.value;
                                           });

    event_manager.emit(int_event{ 7 });
    std::cout << value << std::endl; // 0

    event_manager.connect(event_box);
    event_manager.emit(int_event{ 7 });
    std::cout << value << std::endl; // 0

    other_event_manager.emit(event_box);
    std::cout << value << std::endl; // 7

    return EXIT_SUCCESS;
}
