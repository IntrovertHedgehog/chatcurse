ui thread process input the moment it receive it (not through event queue)
tg thread push new event into event queue and wait for the main thread to process
