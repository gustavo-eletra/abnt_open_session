#include <stdio.h>
#include <ABNTSession.h>

typedef enum
{
    OPEN_SESSION,
    SEND_COMMAND,
}COMMANDS;

void app_main(void)
{
    setup_uart();
    CommandQueue *cq = create_command_queue(10);
    UARTData *t_data = uart_data_as_abnt();
    register_func_to_command_queue(cq, &open_abnt_session, (void*)t_data, sizeof(t_data), OPEN_SESSION);
    enqueue_command_buffer(cq, OPEN_SESSION);
    process_command_queue(cq);
}