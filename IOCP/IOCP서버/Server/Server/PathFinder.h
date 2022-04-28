#pragma once

class Node {
public:
	int x, y;
	int f, g, h;

	Node* pre;
	Node() {
		h = g = f = 0;
		x = y = 0;
		pre = nullptr;
	}
	void init() {
		h = f = 0;
		x = y = 0;
		g = 9999;
		pre = nullptr;
	}
	Node(int _y, int _x) {
		x = _x;
		y = _y;
		pre = nullptr;
	}

	bool operator==(const Node& rhs) {
		if (rhs.x != x) return false;
		if (rhs.y != y) return false;
		return true;
	}

};
typedef std::pair<int, Node*> fweight;

const int dx[4] = { 1,-1, 0,0 };
const int dy[4] = { 0, 0,-1,1 };

const int MAX_DEPTH = 7;
constexpr int MAX_LENGTH = MAX_DEPTH * 2;
constexpr int MAX_STEP = MAX_LENGTH * MAX_LENGTH;


class PathFinder
{
	Node* nodes = nullptr;

	std::vector<Node> Close;
	std::vector<Node*> Open;
	char* p_map_data;
public:
	PathFinder(char* _p_map_data);
	~PathFinder();

	int find_path(GameObject* finder, const GameObject* const target);
	static int cal_h(int end_x, int end_y, int next_x, int next_y);
};

