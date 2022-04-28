#include"stdfx.h"
#include "Network.h"
#include "PathFinder.h"

int PathFinder::cal_h(int end_x, int end_y, int next_x, int next_y) { //맨해튼 거리 사용
	int x = abs(end_x - next_x);
	int y = abs(end_y - next_y);
	return (x + y) * 10;
}
PathFinder::PathFinder(char* _p_map_data) {
	p_map_data = _p_map_data;
	nodes = new Node[MAX_LENGTH * MAX_LENGTH];
}
PathFinder::~PathFinder() {
	delete[] nodes;
}
int PathFinder::find_path(GameObject* finder, const GameObject* const target) {
	int cur_finder_x = finder->x;
	int cur_finder_y = finder->y;
	for (int i = -MAX_DEPTH; i < MAX_DEPTH; ++i) {
		for (int j = -MAX_DEPTH; j < MAX_DEPTH; ++j) {
			int x = cur_finder_x + i;
			int y = cur_finder_y + j;
			if (x < 0 || y < 0) Close.emplace_back(MAX_DEPTH + j, MAX_DEPTH + i);
			else if (x > WORLD_WIDTH || y > WORLD_HEIGHT) Close.emplace_back(MAX_DEPTH + j, MAX_DEPTH + i);
			else if (p_map_data[y * WORLD_HEIGHT + x] == 0) Close.emplace_back(MAX_DEPTH + j, MAX_DEPTH + i);
		}
	}
	for (int i = 0; i < MAX_LENGTH * MAX_LENGTH; ++i) {
		nodes[i].init();
	}
	for (int i = 0; i < MAX_LENGTH; ++i) {
		Close.emplace_back(0, i);
		Close.emplace_back(i, 0);
		Close.emplace_back(i, MAX_LENGTH);
		Close.emplace_back(MAX_LENGTH, i);
	}
	//가까운 오브젝트만 find_path를 호출함 따로 검사하지 않는다.
	int target_x = target->x - cur_finder_x;
	int target_y = target->y - cur_finder_y;

	Node end(MAX_DEPTH + target_y, MAX_DEPTH + target_x);

	nodes[MAX_LENGTH * MAX_DEPTH + MAX_DEPTH].x = MAX_DEPTH;
	nodes[MAX_LENGTH * MAX_DEPTH + MAX_DEPTH].y = MAX_DEPTH;

	Node* now = &nodes[MAX_LENGTH * MAX_DEPTH + MAX_DEPTH];
	Open.push_back(now);

	auto pre_time = std::chrono::system_clock::now();
	int step = 0;
	now->g = 0;
	while (!Open.empty())//|| std::chrono::milliseconds(1) < std::chrono::system_clock::now() - pre_time) {
	{
		auto pass = max_element(Open.cbegin(), Open.cend(), [&](Node* rhs, Node* lhs) {
			return rhs->f > rhs->f;
			});
		now = *pass;
		Open.erase(pass);
		step++;
		if (*now == end || step > MAX_STEP) {
			//std::cout << finder->index << "가 경로 찾음\n";
			break;
		}
		Close.emplace_back(now->y, now->x);// 닫힌 경로에 추가하고


		int cur_y = now->y;
		int cur_x = now->x;

		//시작점에서 인접한 사각형 중 이동가능한 점을 확인
		for (int i = 0; i < 4; ++i) {
			int next_py = cur_y + dy[i];
			int next_px = cur_x + dx[i];
			if (next_px < 0 || next_py < 0) continue;
			if (next_px > MAX_LENGTH || next_py > MAX_LENGTH) continue;

			auto p = find(Close.begin(), Close.end(), Node(next_py, next_px));
			if (p != Close.end()) continue;

			Node* n = &nodes[MAX_LENGTH * next_py + next_px];
			if (n->g < now->g + 10) continue;

			n->x = next_px;
			n->y = next_py;

			n->g = now->g + 10;
			n->h = cal_h(end.x, end.y, n->x, n->y);
			n->f = n->g + n->h;
			n->pre = now;


			Open.push_back(n);

		}
	}
	Close.clear();
	Open.clear();
	if (now->pre == nullptr) return -1;
	if (step > MAX_STEP) {
		reinterpret_cast<Npc*>(finder)->target_id = -1;
		return rand() % 4;
	}
	//std::cout << now->x << ", " << now->y << std::endl;
	while (now->pre->pre != nullptr) {
		now = now->pre;
		//std::cout << now->x << ", " << now->y << std::endl;
	}

	// 0 : up,  1: down, 2:left, 3:right
	if (now->x > MAX_DEPTH) return 3;
	if (now->x < MAX_DEPTH) return 2;

	if (now->y < MAX_DEPTH) return 0;
	if (now->y > MAX_DEPTH) return 1;
	return -1;
}