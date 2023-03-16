#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <math.h>
#include <string>

using namespace std;
using namespace sf;

const int WINDOW_HEIGHT = 1080, WINDOW_WIDTH = 1920;
const double pi = 3.1415926535;

Font font;

double RadToDeg(double x) {return x * 180 / pi;}

int q(int x)
{
    if(x == 0) {return 1;}
    int cnt = 0;
    while(x != 0) {
        x /= 10;
        ++cnt;
    }
    return cnt;
}

class visualKnot
{
private:
    CircleShape cell_;
    Color color_;
    Text text_;
public:
    visualKnot(char numb[]) {
        color_ = Color(255, 165, 0);
        cell_.setFillColor(color_);
        cell_.setOutlineColor(Color::Cyan);
        cell_.setOutlineThickness(4);
        cell_.setRadius(50);
        cell_.setOrigin(cell_.getRadius(), cell_.getRadius());
        text_.setColor(Color::Blue);
        text_.setFont(font);
        text_.setString(numb);
    }
    CircleShape getCell() {return cell_;}
    Text getText() {return text_;}
    RectangleShape* update(int X, bool is_left, int level, int cnt, int q);
};

RectangleShape* visualKnot::update(int X, bool is_left, int level, int cnt, int q)
{
    double r = min(80., 80. / q * (8. / 5));
    text_.setCharacterSize(r);
    if(level == 0) {
        cell_.setPosition(X, cell_.getRadius() + 20 + level * 120);
        text_.setPosition(cell_.getPosition().x - (q / 2.) * (11. / 16) * r, cell_.getPosition().y - (5. / 4) * r / 2);
        return 0;
    }
    int dist = cell_.getRadius();
    for(int i = cnt - 1; i >= level; --i) {
        dist = 20 + 2 * dist;
    }
    if(is_left) {
        cell_.setPosition(X - 10 - dist / 2, cell_.getRadius() + 20 + level * (100 + cnt * 50));
    }
    else {
        cell_.setPosition(X + 10 + dist / 2, cell_.getRadius() + 20 + level * (100 + cnt * 50));
    }
    text_.setPosition(cell_.getPosition().x - (q / 2.) * (11. / 16) * r, cell_.getPosition().y - (5. / 4) * r / 2);
    double d = sqrt((X - cell_.getPosition().x) * (X - cell_.getPosition().x) + (100 + cnt * 50) * (100 + cnt * 50));
    double x = (cell_.getPosition().x - X) / d;
    double deg = RadToDeg(acos(x));
    RectangleShape *line = new RectangleShape(sf::Vector2f(d, 4));
    line->rotate(deg);
    line->setPosition(X, cell_.getRadius() + 20 + (level - 1) * (100 + cnt * 50));
    line->setFillColor(Color::Cyan);
    return line;
}

struct knot
{
    int key_;
    int height;
    int level_;
    visualKnot *body;
    knot *left, *right;
    knot(int val, int level) {
        level_ = level;
        key_ = val;
        left = right = NULL;
        char numb[10] = "";
        itoa(val, numb, 10);
        body = new visualKnot(&numb[0]);
    }
    knot* littleRightRotate();
    knot* littleLeftRotate();
};

int heightSearch(knot *prev, knot *p, bool is_left, int max)
{
    if(p != NULL) {
        if(prev == NULL) {
            p->level_ = 0;
        }
        else if(is_left) {
            p->level_ = prev->level_ + 1;
        }
        else {
            p->level_ = prev->level_ + 1;
        }
        max = heightSearch(p, p->left, true, max);
        max = heightSearch(p, p->right, false, max);
    }
    else if(prev != NULL && prev->level_ > max) {return prev->level_;}
    return max;
}

vector<pair<CircleShape, Text> > k;

void update(knot *prev, knot *p, bool is_left, RenderWindow &window, int levelsCnt)
{
    if(p != NULL) {
        if(prev == NULL) {p->body->update(WINDOW_WIDTH / 2, false, p->level_, levelsCnt, q(p->key_));}
        else {
            RectangleShape *line = p->body->update(prev->body->getCell().getPosition().x, is_left, p->level_, levelsCnt, q(p->key_) + (p->key_ < 0));
            window.draw(*line);
            delete line;
        }
        k.push_back(make_pair(p->body->getCell(), p->body->getText()));
        update(p, p->left, true, window, levelsCnt);
        update(p, p->right, false, window, levelsCnt);
    }
}

int height(knot* p)
{
    return (p != NULL)?p->height:0;
}

int bfactor(knot* p)
{
    return height(p->right) - height(p->left);
}

void fixheight(knot *p)
{
    int hl = height(p->left);
    int hr = height(p->right);
    p->height = ((hl > hr)?hl:hr) + 1;
}

knot* knot::littleRightRotate()
{
    knot* q = this->left;
    this->left = q->right;
    q->right = this;
    fixheight(this);
    fixheight(q);
    return q;
}

knot* knot::littleLeftRotate()
{
    knot* q = this->right;
    this->right = q->left;
    q->left = this;
    fixheight(this);
    fixheight(q);
    return q;
}

knot* balance(knot *p)
{
	fixheight(p);
	if(bfactor(p) == 2) {
		if(bfactor(p->right) < 0) {
			p->right = p->right->littleRightRotate();
		}
		return p->littleLeftRotate();
	}
	if(bfactor(p) == -2) {
		if(bfactor(p->left) > 0) {
			p->left = (p->left)->littleLeftRotate();
		}
		return p->littleRightRotate();
	}
	return p;
}

knot* insert(knot *p, int val)
{
    if(p == NULL) {
        knot *t = new knot(val, 0);
        p = t;
        fixheight(p);
    }
    else if(val < p->key_) {
        if(p->left == NULL) {
            knot *t = new knot(val, p->level_ + 1);
            p->left = t;
            fixheight(p->left);
        }
        else {p->left = insert(p->left, val);}
    }
    else if(val > p->key_) {
        if(p->right == NULL) {
            knot *t = new knot(val, p->level_ + 1);
            p->right = t;
            fixheight(p->right);
        }
        else {p->right = insert(p->right, val);}
    }
    return balance(p);
}

knot* findMin(knot* p)
{
	return p->left?findMin(p->left):p;
}

knot* removeMin(knot* p)
{
	if(p->left == NULL) {return p->right;}
	p->left = removeMin(p->left);
	return balance(p);
}

knot* remove(knot* p, int k)
{
	if(p == NULL) {return 0;}
	if(k < p->key_) {
		p->left = remove(p->left, k);
	}
	else if(k > p->key_) {
		p->right = remove(p->right, k);
	}
	else {
		knot* q = p->left;
		knot* r = p->right;
		delete p;
		if(r == NULL) {return q;}
		knot* min = findMin(r);
		min->right = removeMin(r);
		min->left = q;
		return balance(min);
	}
	return balance(p);
}

int main()
{
    srand(time(NULL));
    string j;
    knot *top = NULL;
    string s = "";
    while(s != "y" && s != "n") {
        cout << "random? (y/n)\n";
        cin >> s;
        if(s == "y") {
            cout << "how many?\n";
            int n;
            cin >> n;
            for(int i = 0; i < n; ++i) {
                int v = rand();
                top = insert(top, v);
            }
        }
        system("cls");
    }
    cout << "waiting...";

    bool add = false, del = false;

    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "AVL tree", Style::Default);
    window.setPosition(Vector2i(-9, -37));
    font.loadFromFile("a_Futurica_ExtraBold.ttf");

    View view;
    view.setCenter(Vector2f(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2));
    view.setSize(Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    window.setView(view);

    Text text;
    text.setFont(font);
    text.setString("Press \"I\" to insert new one.\n"
                   "Press \"R\" to remove current.");
    text.setCharacterSize(20);
    text.setFillColor(Color::Green);

    Clock clock;
    float time;
    int f = 0;
    Text FPS;
    FPS.setFont(font);

    while (window.isOpen()) {
        time = clock.getElapsedTime().asSeconds();
        if(time >= 1) {
            clock.restart();
            char fps[5] = "";
            itoa(f, fps, 10);
            FPS.setString(&fps[0]);
            FPS.setFillColor(Color::Black);
            f = 0;
        }

        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::Escape)
                    window.close();
            }
            if (Keyboard::isKeyPressed(Keyboard::Up)) {
                view.zoom(1.0300f);
                text.scale(1.0300f, 1.0300f);
                FPS.scale(1.0300f, 1.0300f);
                window.setView(view);
            }
            if (Keyboard::isKeyPressed(Keyboard::Down)) {
                view.zoom(0.9699f);
                text.scale(0.9699f, 0.9699f);
                FPS.scale(0.9699f, 0.9699f);
                window.setView(view);
            }
            if (Keyboard::isKeyPressed(Keyboard::W)) {
                view.move(0, -5 * view.getSize().x / WINDOW_WIDTH);
                window.setView(view);
            }
            if (Keyboard::isKeyPressed(Keyboard::A)) {
                view.move(-5 * view.getSize().x / WINDOW_WIDTH, 0);
                window.setView(view);
            }
            if (Keyboard::isKeyPressed(Keyboard::S)) {
                view.move(0, 5 * view.getSize().x / WINDOW_WIDTH);
                window.setView(view);
            }
            if (Keyboard::isKeyPressed(Keyboard::D)) {
                view.move(5 * view.getSize().x / WINDOW_WIDTH, 0);
                window.setView(view);
            }
            if (event.type == Event::KeyPressed) {
                if(event.key.code == Keyboard::R) {
                    system("cls");
                    cout << "enter key: ";
                    del = true;
                }
                if(event.key.code == Keyboard::I) {
                    system("cls");
                    cout << "enter key: ";
                    add = true;
                }
            }
        }

        window.clear(Color(128, 128, 128));
        text.setPosition(view.getCenter().x - view.getSize().x / 2, view.getCenter().y - view.getSize().y / 2);
        int levelsCnt = heightSearch(NULL, top, 0, 0) + 1;
        k.clear();
        update(NULL, top, false, window, levelsCnt);
        window.draw(text);
        for(vector<pair<CircleShape, Text> >::iterator it = k.begin(); it != k.end(); ++it) {
            window.draw((*it).first);
            window.draw((*it).second);
        }
        FPS.setPosition(view.getCenter().x - view.getSize().x / 2 + view.getSize().x * 0.96, view.getCenter().y - view.getSize().y / 2);
        window.draw(FPS);
        window.display();
        ++f;

        if(del) {
            cin >> j;
            int v = 0;
            bool flag = true, minus = false;
            for(int i = 0; i < j.size(); ++i) {
                if(j[i] <= '9' && j[i] >= '0' || (i == 0 && j[i] == '-')) {
                    if(j[i] == '-') {minus = true;}
                    else {
                        v *= 10;
                        v += j[i] - '0';
                    }
                }
                else {
                    flag = false;
                    break;
                }
            }
            if(minus) {v *= -1;}
            if(flag) {top = remove(top, v);}
            system("cls");
            cout << "waiting...";
            del = false;
        }
        else if(add) {
            cin >> j;
            int v = 0;
            bool flag = true, minus = false;;
            for(int i = 0; i < j.size(); ++i) {
                if((j[i] <= '9' && j[i] >= '0') || (i == 0 && j[i] == '-')) {
                    if(j[i] == '-') {minus = true;}
                    else {
                        v *= 10;
                        v += j[i] - '0';
                    }
                }
                else {
                    flag = false;
                    break;
                }
            }
            if(minus) {v *= -1;}
            if(flag) {top = insert(top, v);}
            system("cls");
            cout << "waiting...";
            add = false;
        }
    }

    return 0;
}
