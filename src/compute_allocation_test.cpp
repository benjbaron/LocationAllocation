//
// Created by Benjamin Baron on 17/02/16.
//

#include "compute_allocation.h"
#include <queue>


/* To compile this file and execute the main below, add this file to the CMakeList and remove the main.cpp */
struct comparePairs {
    bool operator()(const QPair<double, QString>& l, const QPair<double, QString>& r)
    {
        return l.first > r.first;
    }
};


void updateTopC(QList<QPair<double,QString>> *c, double coverage, QString id) {
    qDebug() << ">>  add candidate" << id << coverage;
    qDebug() << "--- print the content of the top candidates (before) ---";
    int i = 0;
    for(QPair<double, QString> a : *c) {
        qDebug() << i++ << a.second << a.first;
    }

    if(c->size() < 5 || coverage > c->last().first) {
        // go through the top candidates from the bottom of the list
        // and insert the current one at the right index
        QPair<double, QString> p(coverage,id);
        int idx = 0;
        while(idx < c->size()-1 && coverage < c->at(idx).first) {
            idx++;
            qDebug() << "idx" << idx << coverage << c->at(idx).first;
        }
        qDebug() << "end of loop" << idx;

        if(!c->isEmpty() && coverage < c->at(idx).first) c->push_back(p);
        else c->insert(idx, p);

        if(c->size() > 5) c->pop_back(); // keep the top 5 candidates
    }

    qDebug() << "--- print the content of the top candidates (after) ---";
    i = 0;
    for(QPair<double, QString> a : *c) {
        qDebug() << i++ << a.second << a.first;
    }
}

int main(int argc, char *argv[]) {
    QList<QPair<double,QString>> topCandidates;
    std::priority_queue<int,QList<QPair<double,QString>>, comparePairs > pq;
    pq.push(QPair<double, QString>(1.5,"a"));
    pq.push(QPair<double, QString>(4.5,"b"));
    pq.push(QPair<double, QString>(3.5,"c"));
    pq.push(QPair<double, QString>(0.5,"d"));
    pq.push(QPair<double, QString>(2.5,"e"));
    pq.push(QPair<double, QString>(0.0,"f"));
    pq.push(QPair<double, QString>(9.0,"g"));

    while ( !pq.empty() ) {
        qDebug() << pq.top();
        pq.pop();
    }

    updateTopC(&topCandidates,1.5,"a");
    updateTopC(&topCandidates,4.5,"b");
    updateTopC(&topCandidates,3.5,"c");
    updateTopC(&topCandidates,0.5,"d");
    updateTopC(&topCandidates,2.5,"e");
    updateTopC(&topCandidates,0.0,"f");
    updateTopC(&topCandidates,9.0,"g");
    updateTopC(&topCandidates,8.0,"h");
    updateTopC(&topCandidates,-1.0,"i");
}

