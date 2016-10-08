//
// Created by Appolinariya on 06.10.16.
//

#include <iostream>
#include "persistent_set.h"

persistent_set::node::node() : value(NULL) {
    left = NULL;
    right = NULL;
}

persistent_set::node::node(value_type v) : value(v) {
    left = NULL;
    right = NULL;
}

persistent_set::node::node(value_type v, std::shared_ptr<const node> l, std::shared_ptr<const node> r) : value(v), left(l), right(r) {
}

persistent_set::node::~node() {
}

bool persistent_set::node::find(std::vector<const node *> & path, value_type key) const {
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

std::shared_ptr<const persistent_set::node> persistent_set::node::add(value_type key) const {
    if (value > key) {
        if (left == NULL) {
            std::shared_ptr<const node> l = (std::shared_ptr<const node>)new node(key);
            std::shared_ptr<const node> p = (std::shared_ptr<const node>)new node(value, l, right);
        return p;
        } else {
            std::shared_ptr<const node> l = left->add(key);
            std::shared_ptr<const node> p = (std::shared_ptr<const node>)new node(value, l, right);
            return p;
        }
    } else {
        if (right == NULL) {
            std::shared_ptr<const node> r = (std::shared_ptr<const node>)new node(key);
            std::shared_ptr<const node> p = (std::shared_ptr<const node>)new node(value, left, r);
            return p;
        } else {
            std::shared_ptr<const node> r = right->add(key);
            std::shared_ptr<const node> p = (std::shared_ptr<const node>)new node(value, left, r);
            return p;
        }
    }
}

std::shared_ptr<const persistent_set::node> persistent_set::node::del(std::vector <const node *> & path, iterator it) const {
    path.push_back(this);
    value_type m = *it;
    if (value == m) {
        if (iterator(path, false) == it) {
            return this->down();
        } else {
            return (std::shared_ptr<const node>)new node();
        }
    } else if (value > m) {
        if (left != NULL) {
            std::shared_ptr<const node> l = left->del(path, it);
            std::shared_ptr<const node> p = (std::shared_ptr<const node>)new node(value, l, right);
            return p;
        } else {
            return (std::shared_ptr<const node>)new node();
        }
    } else {
        if (right != NULL) {
            std::shared_ptr<const node> r = right->del(path, it);
            std::shared_ptr<const node> p = (std::shared_ptr<const node>)new node(value, left, r);
            return p;
        } else {
            return (std::shared_ptr<const node>)new node();
        }
    }
}

std::shared_ptr<const persistent_set::node> persistent_set::node::down() const {
    if (left == NULL && right == NULL) {
        delete this;
        return NULL;
    } else if (left == NULL)
        return right;
    else if (right == NULL)
        return left;
    else {
        value_type t = left->value;
        std::shared_ptr<const node> l = left->down();
        std::shared_ptr<const node> p = (std::shared_ptr<const node>)new node(t, l, right);
        return p;
   }
}

void persistent_set::node::begin(std::vector<const node *> &path) const {
    path.push_back(this);
   if (left == NULL)
        return;
    left->begin(path);
}

void persistent_set::node::end(std::vector<const node *> &path) const {
    path.push_back(this);
    if (right != NULL)
        right->end(path);
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
    roots = NULL;
}

persistent_set::iterator persistent_set::find(value_type key) {
    std::vector <const node *> path;
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
        roots = (std::shared_ptr<const node>)new node(key);
    return std::make_pair(find(key), true);
}

void persistent_set::erase(iterator it) {
    std::vector<const node *> path;
    roots = roots->del(path, it);
}

persistent_set::iterator persistent_set::begin() const {
        std::vector <const node *> path;
        roots->begin(path);
        return iterator(path, false);
    }

persistent_set::iterator persistent_set::end() const {
        std::vector <const node *> path;
        roots->end(path);
        return iterator(path, true);
    }

persistent_set::iterator::iterator() : path(), flag_end(false) {
}

persistent_set::iterator::iterator(iterator const &other) : path(other.path), flag_end(other.flag_end) {
}

persistent_set::iterator::iterator(std::vector<const node *> p, bool f) : path(p), flag_end(f) {
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
    std::vector <const node *> p = path;
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
    std::vector <const node *> p = path;
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