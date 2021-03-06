#include "thread_pool.h"

ThreadPool::ThreadPool() {

	conn_lock = PTHREAD_MUTEX_INITIALIZER;
	conn_req = PTHREAD_COND_INITIALIZER;
	conn_full = PTHREAD_COND_INITIALIZER;
	clear_connections();
}

ThreadPool::ThreadPool(const ThreadPool &src_obj) {

	conn_lock = src_obj.conn_lock;
	conn_req = src_obj.conn_req;
	conn_full = src_obj.conn_full;
	connections = src_obj.connections;
	max_threads = src_obj.max_threads;
	thread_num = src_obj.thread_num;
	thread_id = src_obj.thread_id;
	thread_func = src_obj.thread_func;
}

void swap(ThreadPool &src_obj, ThreadPool &dst_obj) {
	using std::swap;

	swap(src_obj.conn_lock, dst_obj.conn_lock);
	swap(src_obj.conn_req, dst_obj.conn_req);
	swap(src_obj.conn_full, dst_obj.conn_full);
	swap(src_obj.connections, dst_obj.connections);
	swap(src_obj.max_threads, dst_obj.max_threads);
	swap(src_obj.thread_num, dst_obj.thread_num);
	swap(src_obj.thread_id, dst_obj.thread_id);
	swap(src_obj.thread_func, dst_obj.thread_func);
}

ThreadPool& ThreadPool::operator=(ThreadPool src_obj) {

	swap(*this, src_obj);
	return *this;
}

ThreadPool::ThreadPool(ThreadPool &&src_obj)
	:ThreadPool() {

	swap(*this, src_obj);
}

void ThreadPool::clear_connections() {

	while (!connections.empty())
		connections.pop();	
}

void ThreadPool::set_max_threads(int count) {

	max_threads = count; 
	thread_num.resize(max_threads);
	thread_id.resize(max_threads);	
}

void ThreadPool::set_thread_func(void*(*thread_func)(void*)) {
	
	this->thread_func = thread_func;
}

void ThreadPool::create_threads() {
	int i;
	int status;

	for (i = 0; i < max_threads; i++) {
		status = pthread_create(&thread_id[i], NULL, thread_handler, this);
		report_error(status, "Error in creating threads");
	}
}

ClientDetails ThreadPool::fetch_connection() {
	ClientDetails entity;

	if (!connections.empty()) {
		entity = connections.front();
		connections.pop();
	}
	else {
		entity.num = -1;
	}
	return entity;
}

ThreadPool::~ThreadPool() {

	
}

void* thread_handler(void *arg) {
	struct ThreadPool *tpool;
	ClientDetails entity;
	int num;
	int status;
	void *conn_info;

	tpool = ((struct ThreadPool*)arg);
	while (1) {
		status = pthread_mutex_lock(&(tpool->conn_lock));
		report_error(status, "Error in locking");
		while ((entity = tpool->fetch_connection()).num == -1) {
			status = pthread_cond_wait(&(tpool->conn_req), &(tpool->conn_lock));
			report_error(status, "Conditional wait failed");
		}
		status = pthread_cond_signal(&(tpool->conn_full));
		report_error(status, "Error in signalling event");
		status = pthread_mutex_unlock(&(tpool->conn_lock));
		conn_info = &entity;
		(*(tpool->thread_func))(conn_info);
	}	
}
