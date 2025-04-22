#include "factory.h"
#include <iostream>
#include <memory>
using namespace std;
using namespace cspp51045;

struct Scrollbar {
  virtual size_t position() = 0;
};

struct Button {
  virtual void press() = 0;
};

struct QtScrollbar : public Scrollbar {
  size_t position() { return 0; }
};

struct QtButton : public Button {
  void press() { cout << "QtButton pressed" << endl; }
};

struct WindowsScrollbar : public Scrollbar {
  size_t position() { return 0; }
};

struct WindowsButton : public Button {
  void press() { cout << "WindowsButton pressed" << endl; }
};

using AbstractWidgetFactory = abstract_factory<Scrollbar, Button>;
using QtWidgetFactory 
  = concrete_factory<AbstractWidgetFactory, QtScrollbar, QtButton>;
using WindowsWidgetFactory
  = concrete_factory<AbstractWidgetFactory, WindowsScrollbar, WindowsButton>;

int main()
{
  unique_ptr<AbstractWidgetFactory> factory 
	  = make_unique<WindowsWidgetFactory>();
  unique_ptr<Button> button(factory->create<Button>());
  button->press();
  return 0;
}
