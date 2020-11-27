#include "TXLib.h"

#include <iostream> 
#include <algorithm>
#include <ctime>
#include <fstream>
#include <string.h>

typedef double (*func_t) (double x);

class Window {
private:
	int x0_, y0_, x1_, y1_;
	const char* text_;

	void grid();
	void axes();
public:
	void draw_field();
	void draw_text();

	Window(int x0, int y0, int x1, int y1, const char* text) {
		x0_ = x0;
		y0_ = y0;
		x1_ = x1;
		y1_ = y1;
		text_ = text;
	}
};


class CoordSys {
private:
	int x0_, y0_, x1_, y1_;
	double scaleX_, scaleY_;



public:
	void draw_point(double x, double y);
	double find_x(double x);
	double find_y(double y);

	CoordSys(int x0, int y0, int x1, int y1, double scaleX, double scaleY) {
		x0_ = x0;
		y0_ = y0;
		x1_ = x1;
		y1_ = y1;
		scaleX_ = scaleX;
		scaleY_ = scaleY;
	}
};


class Button {
public:
	const char* name_;
	RECT rect_;
	func_t func_;

	Button(const char* name, RECT rect, func_t func) :
		name_(name),
		rect_(rect),
		func_(func)
	{};

	virtual void draw_button() {
		txSetColor(TX_RED);
		txLine(rect_.left, rect_.top, rect_.right, rect_.bottom);
		txLine(rect_.left, rect_.bottom, rect_.right, rect_.top);
		txSetColor(TX_WHITE);
	}

	virtual int is_button_pressed()  {
		double x = txMouseX(), y = txMouseY();

		if (x == rect_.right + 0.5 * (rect_.right - rect_.left) && y == rect_.top + 0.5 * (rect_.bottom - rect_.top)) return 1;

		return 0;
	}

	void draw_graph(CoordSys& field);
};

class RectButton : public Button {
public:
	RectButton(const char* name, RECT rect, func_t func) :
		Button(name, rect, func) {}

	virtual void draw_button() override {
		txSetFillColor(TX_WHITE);
		txRectangle(rect_.left, rect_.top, rect_.right, rect_.bottom);

		txSetColor(TX_BLACK);
		txDrawText(rect_.left, rect_.top, rect_.right, rect_.bottom, name_);
	}

	virtual int is_button_pressed() override {
		double x = txMouseX(), y = txMouseY();

		if (x >= rect_.left && x <= rect_.right && y >= rect_.top && y <= rect_.bottom) return 1;

		return 0;
	}
};

class CircleButton : public Button {
public:
	CircleButton(const char* name, RECT rect, func_t func) :
		Button(name, rect, func) {}

	virtual void draw_button() override {
		txSetFillColor(TX_WHITE);
		txEllipse(rect_.left, rect_.top, rect_.right, rect_.bottom);

		txSetColor(TX_BLACK);
		txDrawText(rect_.left, rect_.top, rect_.right, rect_.bottom, name_);
	}

	virtual int is_button_pressed() override {
		double R = 0.5 * (rect_.right - rect_.left);
		double x = txMouseX(), y = txMouseY(), x0 = rect_.left + R, y0 = rect_.top + R;

		if ((x - x0) * (x - x0) + (y - y0) * (y - y0) <= R * R)	return 1;

		return 0;
	}
};

class EllipseButton : public Button {
public:
	EllipseButton(const char* name, RECT rect, func_t func) :
		Button(name, rect, func) {}

	virtual void draw_button() override {
		txSetFillColor(TX_WHITE);
		txEllipse(rect_.left, rect_.top, rect_.right, rect_.bottom);

		txSetColor(TX_BLACK);
		txDrawText(rect_.left, rect_.top, rect_.right, rect_.bottom, name_);
	}

	virtual int is_button_pressed() override {
		double a = 0.5 * (rect_.right - rect_.left), b = 0.5 * (rect_.bottom - rect_.top);
		double x = txMouseX(), y = txMouseY(), x0 = rect_.left + a, y0 = rect_.top + b;

		if ((x - x0) * (x - x0) / (a * a) + (y - y0) * (y - y0) / (b * b) <= 1) return 1;

		return 0;
	}
};

const size_t Number_of_buttons = 50;

class Manager {
public:
	size_t count_ = 0;
	Button* buttons_[Number_of_buttons] = {};

	Manager() {
		for (size_t button = 0; button < Number_of_buttons; button++)
			buttons_[button] = NULL;
	}

	void add(Button* button);
	void draw(Window& field);
	void run(Window& field, CoordSys& graph);
};

void CoordSys::draw_point(double x0, double y0) {
	double x = x0 * scaleX_, y = y0 * scaleY_;

	if (find_x(x) > x0_ && find_x(x) < x1_ && find_y(y) < y1_ && find_y(y) > y0_)
		txCircle(find_x(x), find_y(y), 1);
}

double CoordSys::find_x(double x) {
	return (x1_ - x0_) / 2 + x;
}

double CoordSys::find_y(double y) {
	return (y1_ - y0_) / 2 - y;
}

void Window::draw_text() {
	txSelectFont("System", false, false, false, false, false, false, 0);

	txSetColor(TX_WHITE);
	txRectangle(x0_ + 50, y1_ + 10, x1_, y1_ + 40);

	txSetColor(TX_BLACK);
	txDrawText(x0_, y1_ + 10, x1_, y1_ + 40, text_);
}

void Window::grid() {
	txSetColor(TX_LIGHTGRAY);

	for (int x = x0_; x < x1_; x += 10) txLine(x, y0_, x, y1_);
	for (int y = y0_ + 5; y < y1_; y += 10) txLine(x0_, y, x1_, y);
}

void Window::axes() {
	txSetColor(TX_BLACK);

	txLine(x0_, (y1_ - y0_) / 2, x1_, (y1_ - y0_) / 2);
	txLine((x1_ - x0_) / 2, y0_, (x1_ - x0_) / 2, y1_);

	txLine(x1_, (y1_ - y0_) / 2, x1_ - 5, (y1_ - y0_) / 2 - 5);
	txLine(x1_, (y1_ - y0_) / 2, x1_ - 5, (y1_ - y0_) / 2 + 5);

	txLine((x1_ - x0_) / 2, y0_, (x1_ - x0_) / 2 + 5, y0_ + 5);
	txLine((x1_ - x0_) / 2, y0_, (x1_ - x0_) / 2 - 5, y0_ + 5);

	for (int x = x0_ + 30; x < x1_; x += 30) txLine(x, (y1_ - y0_) / 2 - 1, x, (y1_ - y0_) / 2 + 2);
	for (int y = y0_ + 15; y < y1_; y += 30) txLine((x1_ - x0_) / 2 - 1, y, (x1_ - x0_) / 2 + 2, y);
}

void Window::draw_field() {
	txRectangle(x0_, y0_, x1_, y1_);
	draw_text();
	grid();
	axes();
}

//template <size_t N>

void Manager::draw(Window& field) {
	txClear();
	field.draw_field();

	for (size_t button = 0; button < count_; button++)
		buttons_[button]->draw_button();
}

void Button::draw_graph(CoordSys& field) {
	txSetColor(TX_BLACK);

	for (double x = -18; x < 20; x+= 0.01) {
		double y = func_(x);
		field.draw_point(x * 1, y * 1);
	}
}

void Manager::add(Button* button) {
	buttons_[count_] = button;
	count_++;
}

void Manager::run(Window& field, CoordSys& graph) {
	while (!txGetAsyncKeyState(VK_ESCAPE)) {
		if (txMouseButtons() == 1)
			for (size_t button = 0; button < count_; button++)
				if (buttons_[button]->is_button_pressed()) {
					draw(field);
					buttons_[button]->draw_graph(graph);
				}
	}
}

int main() {
	int width = 800, height = 600;
	txCreateWindow(width, height);
	txSetColor(TX_BLACK);

	Window field(20, 20, 780, 450, "");
	CoordSys graph(20, 20, 780, 450, 20, 20);

	Manager manager;

	RectButton    rect_button    ("SIN", RECT{ 100, 480, 300, 520 }, sin); manager.add(&rect_button);
	CircleButton  circle_button  ("COS", RECT{ 175, 540, 225, 590 }, cos); manager.add(&circle_button);
	EllipseButton ellipse_button ("TAN", RECT{ 500, 480, 700, 520 }, tan); manager.add(&ellipse_button);
	Button        button         ("EXP", RECT{ 500, 540, 700, 580 }, exp); manager.add(&button);
	
	manager.draw(field);

	manager.run(field, graph);

	txDisableAutoPause();

}
