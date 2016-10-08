//
// Created by Appolinariya on 06.10.16.
//

#include <iostream>
#include "persistent_set.h"

persistent_set::node::node() : value(NULL), counter(1), change(0) {
    left = NULL;
    right = NULL;
}

persistent_set::node::node(value_type v) : value(v), counter(1), change(0) {
    left = NULL;
    right = NULL;
}

persistent_set::node::node(value_type v, node *l, node *r) : value(v), counter(1), change(0) {
    left = l;
    right = r;
    if (l != NULL)
        l->change++;
    if (r != NULL)
        r->change++;
}

persistent_set::node::~node() {
    if (left != NULL)
        left->delete_();
    if (right != NULL)
        right->delete_();
}

void persistent_set::node::delete_() {
    change--;
    if (change)
        push_change();
    if (!counter)
        this->delete_();
}

bool persistent_set::node::find(std::vector<node *> & path, value_type key) {
    if (change)
        push_change();
    path.push_back(this);
    if (value == key)
        return true;
    if (value > key && left != NULL)
        return left->find(path, key);
    else if (value > key)
        return false;
    if (value < key && right != NULL)
        return right->find(path, key);
    else if (value < key)
        return false;
}

persistent_set::node * persistent_set::node::add(value_type key) {
    if (change)
        push_change();
    if (value > key) {
        if (left == NULL) {
            node *l = new node(key);
            node *p = new node(value, l, right);
            this->delete_();
            return p;
        } else {
            node *l = left->add(key);
            node *p = new node(value, l, right);
            this->delete_();
            return p;
        }
    } else {
        if (right == NULL) {
            node *r = new node(key);
            node *p = new node(value, left, r);
            this->delete_();
            return p;
        } else {
            node *r = right->add(key);
            node *p = new node(value, left, r);
            this->delete_();
            return p;
        }
    }
}

persistent_set::node * persistent_set::node::del(std::vector <node *> & path, iterator it) {
    if (change)
        push_change();
    path.push_back(this);
    value_type m = *it;
    if (value == m) {
        if (iterator(path, false) == it) {
            node *p = this->down();
            this->delete_();
            return p;
        } else {
            return NULL;
        }
    } else if (value > m) {
        if (left != NULL) {
            node *l = left->del(path, it);
            node *p = new node(value, l, right);
            return p;
        } else {
            return NULL;
        }
    } else {
        if (right != NULL) {
            node *r = right->del(path, it);
            node *p = new node(value, left, r);
            return p;
        } else {
            return NULL;
        }
    }
}

persistent_set::node * persistent_set::node::down() {
    if (change)
        push_change();
    if (left == NULL && right == NULL) {
        this->delete_();
        return NULL;
    } else if (left == NULL) {
        node *p = right;
        this->delete_();
        return p;
    } else if (right == NULL) {
        node *p = left;
        this->delete_();
        return p;
    } else {
        value_type t = left->value;
        node *l = left->down();
        node *p = new node(t, l, right);
        this->delete_();
        return p;
    }
}

void persistent_set::node::begin(std::vector<node *> &path) {
    path.push_back(this);
    if (left == NULL)
        return;
    left->begin(path);
}

void persistent_set::node::end(std::vector<node *> &path) {
    path.push_back(this);
    if (right != NULL)
        right->end(path);
}

void persistent_set::node::push_change() {
    counter += change;
    if (left != NULL)
        left->change += change;
    if (right != NULL)
        right->change += change;
    change = 0;
}

persistent_set::persistent_set() : roots(NULL) {
}

persistent_set::persistent_set(persistent_set const &other) : roots(other.roots) {
}

persistent_set& persistent_set::operator=(persistent_set const &other) {
    roots = other.roots;
    return *this;
}

persistent_set::~persistent_set() {
    roots->delete_();
}

persistent_set::iterator persistent_set::find(value_type key) {
    std::vector <node *> path;
    if (roots != NULL) {
        bool f = roots->find(path, key);
        if (!f)
            return end();
        return iterator(path, false);
    }
    return iterator(path, true);
}

std::pair<persistent_set::iterator, bool> persistent_set::insert(value_type key) {
    iterator it = find(key);
    if (!it.flag_end)
        return std::make_pair(it, false);
    if (roots != NULL)
        roots = (*roots).add(key);
    else
        roots = new node(key);
    return std::make_pair(find(key), true);
}

void persistent_set::erase(iterator it) {
    std::vector<node *> path;
    roots = roots->del(path, it);
}

persistent_set::iterator persistent_set::begin() const {
    std::vector <node *> path;
    roots->begin(path);
    return iterator(path, false);
}

persistent_set::iterator persistent_set::end() const {
    std::vector <node *> path;
    roots->end(path);
    return iterator(path, true);
}

persistent_set::iterator::iterator() : path(), flag_end(false) {
}

persistent_set::iterator::iterator(iterator const &other) : path(other.path), flag_end(other.flag_end) {
}

persistent_set::iterator::iterator(std::vector<node *> p, bool f) : path(p), flag_end(f) {
}

persistent_set::iterator& persistent_set::iterator::operator=(const iterator *other) {
    path = other->path;
    flag_end = other->flag_end;
    return *this;
}

persistent_set::value_type const& persistent_set::iterator::operator*() const {
    if (flag_end)
        return NULL;
    return path[path.size() - 1]->value;
}

persistent_set::iterator& persistent_set::iterator::operator++() {
    std::vector <node *> p = path;
    if (flag_end)
        return *this;
    if (path[path.size() - 1]->right == NULL) {
        while (path.size() > 1 && path[path.size() - 1]->value > path[path.size() - 2]->value) {
            path.pop_back();
        }
        if (path.size() > 1) {
            path.pop_back();
            return *this;
        }
        path = p;
        flag_end = true;
        return *this;
    } else {
        path[path.size() - 1]->right->begin(path);
        return *this;
    }
}

persistent_set::iterator persistent_set::iterator::operator++(int) {
    std::vector <node *> p = path;
    if (flag_end)
        return *this;
    if (path[path.size() - 1]->right == NULL) {
        while (path.size() > 1 && path[path.size() - 1]->value > path[path.size() - 2]->value) {
            path.pop_back();
        }
        if (path.size() > 1) {
            path.pop_back();
            return *this;
        }
        path = p;
        flag_end = true;
        return *this;
    } else {
        path[path.size() - 1]->right->begin(path);
        return *this;
    }
}

persistent_set::iterator& persistent_set::iterator::operator--() {
    if (flag_end) {
        flag_end = false;
        return *this;
    }
    if (path[path.size() - 1]->left == NULL) {
        while (path[path.size() - 1]->value < path[path.size() - 2]->value)
            path.pop_back();
        if (path.size() > 1)
            path.pop_back();
        return *this;
    } else {
        path[path.size() - 1]->left->end(path);
        return *this;
    }
}

persistent_set::iterator persistent_set::iterator::operator--(int) {
    if (flag_end) {
        flag_end = false;
        return *this;
    }
    if (path[path.size() - 1]->left == NULL) {
        while (path[path.size() - 1]->value < path[path.size() - 2]->value)
            path.pop_back();
        if (path.size() > 1)
            path.pop_back();
        return *this;
    } else {
        path[path.size() - 1]->left->end(path);
        return *this;
    }
}

bool operator==(persistent_set::iterator tmp1, persistent_set::iterator tmp2) {
    if (tmp1.path.size() != tmp2.path.size() || tmp1.flag_end != tmp2.flag_end)
        return false;
    size_t tm = tmp1.path.size();
    for (size_t i = 0; i < tm; i++)
        if (tmp1.path[i] != tmp2.path[i])
            return false;
    return true;
}

bool operator!=(persistent_set::iterator tmp1, persistent_set::iterator tmp2) {
    return !(tmp1 == tmp2);
}