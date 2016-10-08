//
// Created by Appolinariya on 06.10.16.
//

#ifndef PERSISTENT_SET_PERSISTENT_SET2_H
#define PERSISTENT_SET_PERSISTENT_SET2_H

#include <cstdio>
#include <utility>
#include <vector>
#include <iostream>
#include <memory>

struct persistent_set
{
    typedef int value_type;
    struct iterator;

protected:
    struct node {
        std::shared_ptr<const node> left, right;
        const value_type value;

        node();
        node(value_type v);
        node(value_type v, std::shared_ptr<const node> l, std::shared_ptr<const node> r);
        ~node();
        bool find(std::vector<const node *> & path, value_type key) const;
        std::shared_ptr<const node> add(value_type key) const;
        std::shared_ptr<const node> del(std::vector<const node *> & path, iterator it) const;
        std::shared_ptr<const node> down() const;
        void begin(std::vector<const node *> & path) const;
        void end(std::vector<const node *> & path) const;
    };

    std::shared_ptr<const node> roots;

public:

    persistent_set();
    persistent_set(persistent_set const& other);
    persistent_set& operator=(persistent_set const& other);
    ~persistent_set();
    iterator find(value_type key);
    std::pair<iterator, bool> insert(value_type key);
    void erase(iterator it);
    iterator begin() const;
    iterator end() const;
};

struct persistent_set::iterator
{
    std::vector <const node *> path;
    bool flag_end;
    iterator();
    iterator(iterator const& other);
    iterator(std::vector <const node *> p, bool f);
    iterator& operator=(const iterator *other);
    value_type const& operator*() const;

    iterator& operator++();
    iterator operator++(int);

    iterator& operator--();
    iterator operator--(int);
};

bool operator==(persistent_set::iterator tmp1, persistent_set::iterator tmp2);
bool operator!=(persistent_set::iterator tmp1, persistent_set::iterator tmp2);

#endif //PERSISTENT_SET_PERSISTENT_SET2_H