

int sdl_GetKeysym (int k)
{
	int rc;

	switch (k) {
	case SDLK_UP: rc = KEY_UPARROW; break;
	case SDLK_DOWN: rc = KEY_DOWNARROW; break;
	case SDLK_LEFT: rc = KEY_LEFTARROW; break;
	case SDLK_RIGHT: rc = KEY_RIGHTARROW; break;
	case SDLK_q: rc = KEY_q; break;
	case SDLK_f: rc = KEY_f; break;
	case SDLK_u: rc = KEY_u; break;
	case SDLK_p: rc = KEY_p; break;
	case SDLK_LCTRL: rc = KEY_LCTRL; break;
	case SDLK_SPACE: rc = KEY_SPACEBAR; break;
	case SDLK_TAB: rc = KEY_TAB; break;
	case SDLK_ESCAPE: rc = KEY_ESCAPE; break;
	default: break;
	}

	return rc;
}

void sdl_CheckEventQueue (void)
{
	SDL_Event event;
	event_t myevent;

	// if too many events, use SDL_PeepEvents
	while (SDL_PollEvent(&event))
	{
		switch (event.type) {
	 	case SDL_QUIT:		// clicking x-window close box
			cleanup();
			exit(0);
			break;
		case SDL_KEYDOWN:
			myevent.type = ev_keydown;
			myevent.data1 = I_GetKey(event.key.keysym.sym);
			C_PostEvent(&myevent);
			break;
		case SDL_KEYUP:
			myevent.type = ev_keyup;
			myevent.data1 = I_GetKey(event.key.keysym.sym);
			C_PostEvent(&myevent);
			break;
		default:
			break;
		}
	}
}




