#ifndef I_WINDOW_H
#define I_WINDOW_H


struct WindowConfiguration {
	int width = 0.f;
	int height = 0.f;
	float dpiScale = 1.f;
};

class AbstractWindow {
public:
	virtual ~AbstractWindow() = default;

	virtual void toggleFullscreen() = 0;
	[[nodiscard]] virtual bool isFullscreen() const = 0;
	virtual void close() = 0;

	virtual void updateWindowConfiguration() = 0;

	WindowConfiguration config{};
};


#endif // I_WINDOW_H
