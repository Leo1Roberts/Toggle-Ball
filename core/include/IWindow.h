#ifndef I_WINDOW_H
#define I_WINDOW_H


class IWindow {
public:
	virtual ~IWindow() = default;

	virtual void toggleFullscreen() = 0;
	[[nodiscard]] virtual bool isFullscreen() const = 0;
	virtual void close() = 0;
};


#endif // I_WINDOW_H
