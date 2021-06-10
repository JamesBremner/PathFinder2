#include <sstream>
#include <algorithm>
#include "cPathFinder.h"

std::string cPathFinder::pathViz()
{
    std::string graphvizgraph = "graph";
    std::string graphvizlink = "--";
    if (myGraph.isDirected())
    {
        graphvizgraph = "digraph";
        graphvizlink = "->";
    }

    std::stringstream f;
    f << graphvizgraph << " G {\n";
    for (auto n : myGraph.nodes())
    {
        f << n.second.myName
          << " [color=\"" << n.second.myColor << "\"  penwidth = 3.0 ];\n";
    }

    // loop over links
    for (auto &e : myGraph.links())
    {
        // check if link between two nodes on path
        bool onpath = false;
        auto pathItsrc = std::find(myPath.begin(), myPath.end(), e.first.first);
        auto pathItdst = std::find(myPath.begin(), myPath.end(), e.first.second);
        if (pathItsrc != myPath.end() && pathItdst != myPath.end())
            if (pathItsrc == pathItdst + 1 || pathItsrc == pathItdst - 1)
                onpath = true;

        f << myGraph.node(e.first.first).myName << graphvizlink
          << myGraph.node(e.first.second).myName;
        if (onpath)
            f << "[color=\"red\"] ";
        f << "\n";
    }

    f << "}\n";
    return f.str();
}

std::string cPathFinder::camsViz()
{
    std::string graphvizgraph = "graph";
    std::string graphvizlink = "--";
    if (myGraph.isDirected())
    {
        graphvizgraph = "digraph";
        graphvizlink = "->";
    }

    std::stringstream f;
    f << graphvizgraph << " G {\n";
    for (auto &n : myGraph.nodes())
    {
        f << n.second.myName
          << " [color=\"" << n.second.myColor << "\"  penwidth = 3.0 ];\n";
    }

    // loop over links
    for (auto &e : myGraph.links())
    {
        f << myGraph.node(e.first.first).myName << graphvizlink
          << myGraph.node(e.first.second).myName
          << "\n";
    }

    f << "}\n";
    return f.str();
}

std::string cPathFinder::spanViz(bool all)
{
    std::string graphvizgraph = "graph";
    std::string graphvizlink = "--";
    if (myGraph.isDirected())
    {
        graphvizgraph = "digraph";
        graphvizlink = "->";
    }

    std::stringstream f;
    f << graphvizgraph << " G {\n";
    for (auto &n : myGraph.nodes())
    {
        f << n.second.myName
          << " [color=\"" << n.second.myColor << "\"  penwidth = 3.0 ];\n";
    }

    // loop over links
    for (auto &e : myGraph.links())
    {
        f << myGraph.node(e.first.first).myName << graphvizlink
          << myGraph.node(e.first.second).myName;
        if (mySpanTree.includes_link(e.first))
            f << "[color=\"red\"] ";
        f << "\n";
    }

    f << "}\n";
    return f.str();
}
void cPathFinder::start(const std::string &start)
{
    myStart = find(start);
    if (myStart < 0)
        throw std::runtime_error("cPathFinder::bad start node");
}

// cPathFinder::cEdge &cPathFinder::linkProps(int u, int v)
// {
//     return myLink.at(std::make_pair(u, v));
// }

/** link cost
 * @param[in] u node index
 * @param[in] v node index
 * @return cost of link between u and v
 * If u and v are not adjacent, returns INT_MAX
 */
int cPathFinder::linkCost(int u, int v)
{
    try
    {
        return myGraph.link(u, v).myCost;
    }
    catch (...)
    {
        return INT_MAX;
    }
}

void cPathFinder::path()
{
    int V = nodeCount();

    myDist.clear();
    myDist.resize(V);
    myPred.clear();
    myPred.resize(V, -1);

    bool sptSet[V]; // sptSet[i] will be true if vertex i is included in shortest
    // path tree or shortest distance from src to i is finalized

    // Initialize all distances as INFINITE and stpSet[] as false
    for (int i = 0; i < V; i++)
        myDist[i] = INT_MAX, sptSet[i] = false;

    // Distance of source vertex from itself is always 0
    myDist[myStart] = 0;
    myPred[myStart] = 0;

    // Find shortest path for all vertices
    for (int count = 0; count < V - 1; count++)
    {
        // Pick the minimum distance vertex from the set of vertices not
        // yet processed. u is always equal to src in the first iteration.
        int min = INT_MAX, u;

        for (int v = 0; v < V; v++)
            if (sptSet[v] == false && myDist[v] <= min)
                min = myDist[v], u = v;
        if (min == INT_MAX)
        {
            // no more nodes connected to start
            break;
        }

        // Mark the picked vertex as processed
        sptSet[u] = true;

        // Update dist value of the adjacent vertices of the picked vertex.
        for (int v : myGraph.adjacent(u))
        {
            if (sptSet[v])
                continue; // already processed

            // Update dist[v] only if total weight of path from src to  v through u is
            // smaller than current value of dist[v]
            int cost = linkCost(u, v);
            if (myDist[u] + cost < myDist[v])
            {
                myDist[v] = myDist[u] + cost;
                myPred[v] = u;
            }
        }
    }
    pathPick(myEnd);
}

void cPathFinder::clear()
{
    myGraph.clear();
}

int cPathFinder::find(const std::string &name)
{
    return myGraph.find(name);
}

int cPathFinder::findoradd(const std::string &name)
{
    return myGraph.findoradd(name);
}

std::vector<int> cPathFinder::pathPick(int end)
{
    myPath.clear();
    // std::cout << "->cPathFinder::pathPick "
    //     << myStart <<" " << end << "\n";

    if (end < 0)
        throw std::runtime_error("cPathFinder::pathPick bad end node");
    if (myPred[end] == end || myPred[end] == -1)
        return myPath; // there is no path

    // pick out path, starting at goal and finishing at start
    myPath.push_back(end);
    int prev = end;
    while (1)
    {
        //std::cout << prev << " " << myPred[prev] << ", ";
        int next = myPred[prev];
        myPath.push_back(next);
        if (next == myStart)
            break;
        prev = next;
    }

    // reverse so path goes from start to goal
    std::reverse(myPath.begin(), myPath.end());

    myPathCost = myDist[myPath.back()] + myMaxNegCost * (myPath.size() - 1);

    return myPath;
}

std::string cPathFinder::nodeName(int n)
{
    return myGraph.node(n).myName;
}

void cPathFinder::addLink(
    int u,
    int v,
    double cost)
{
    myGraph.addLink(
        std::to_string(u),
        std::to_string(v),
        cost);
}

void cPathFinder::addLink(
    const std::string &su,
    const std::string &sv,
    double cost)
{
    myGraph.addLink(su, sv, cost);
}

int cPathFinder::linkCount()
{
    return myGraph.linkCount();
}
int cPathFinder::nodeCount()
{
    return myGraph.nodeCount();
}

std::string cPathFinder::linksText()
{
    return myGraph.linksText();
}

std::string cPathFinder::pathText()
{
    std::stringstream ss;
    for (auto n : myPath)
    {
        std::string sn;
        sn = myGraph.node(n).myName;

        if (sn == "???")
            sn = std::to_string(n);
        ss << sn << " -> ";
    }

    if (myPath.size() && myDist.size() && myPathCost >= 0)
    {
        ss << " Cost is "
           << myPathCost + myMaxNegCost * (myPath.size() - 1)
           << "\n";
    }

    return ss.str();
}

void cPathFinder::copyNodes(const graph::cGraph &other)
{
    myGraph.copyNodes(other);
}

void cPathFinder::span()
{
    myPathCost = 0;
    int V = nodeCount();
    std::vector<bool> Q(V, false); // set true when node added to spanning tree
    mySpanTree.clear();            // the spanning tree
    mySpanTree.copyNodes(myGraph);

    // add initial arbitrary link
    int v = 0;
    int w = *myGraph.adjacent(0).begin();
    mySpanTree.addLink(v, w);
    Q[v] = true;
    Q[w] = true;
    std::cout << "adding " << myGraph.name(v) << " " << myGraph.name(w)
              << " " << linkCost(v, w) << "\n";

    // while nodes remain outside of span
    while (1)
    {
        int v;      // node in span
        int w = -1; //node not yet in span
        int min_cost = INT_MAX;

        // loop over nodes in span
        for (int kv = 0; kv < Q.size(); kv++)
        {
            if (!Q[kv])
                continue;

            // loop over nodes not in span
            for (int kw = 0; kw < Q.size(); kw++)
            {
                if (Q[kw])
                    continue;

                if (!myGraph.includes_link(kv, kw))
                    continue;

                // find cheapest link that adds node to span
                int cost = linkCost(kv, kw);
                if (cost < min_cost)
                {
                    min_cost = cost;
                    v = kv;
                    w = kw;
                }
            }
        }

        // check if any nodes were found not yet in span
        if (w == -1)
            break;

        // add node to span
        Q[w] = true;
        mySpanTree.addLink(v, w);
        myPathCost += linkCost(v, w);
    }
}

std::string cPathFinder::spanText()
{
    return mySpanTree.linksText();
}

void cPathFinder::depthFirst(int v)
{
    myPath.clear();
    myPath.resize(nodeCount(), 0);
    myPred.clear();

    depthRecurse(v);
}

void cPathFinder::depthRecurse(int v)
{
    // record new node on the search
    myPred.push_back(v);

    // remember this node has been visted
    myPath[v] = 1;

    // look for new adjacent nodes
    for (int w : myGraph.adjacent(v))
        if (!myPath[w])
        {
            // search from new node
            depthRecurse(w);
        }
}

void cPathFinder::tsp()
{
    // calculate spanning tree
    span();

    // construct pathFinder from spanning tree
    cPathFinder pf;
    pf.myGraph = mySpanTree;

    // depth first search of spanning tree
    pf.depthFirst(0);
    myPath = pf.myPred;

    //return to starting point
    myPath.push_back(myPath[0]);

    // cost
    myPathCost = 0;
    int prev = -1;
    for (int n : myPath)
    {
        if (prev < 0)
        {
            prev = n;
            continue;
        }
        myPathCost += myGraph.link(prev, n).myCost;
        prev = n;
    }

    std::cout << "route " << pathText() << "\n";
}

void cPathFinder::cams()
{
    myPath.clear();

    // working copy on input graph
    auto work = myGraph;

    // The nodes that connect leaf nodes to the rest of the graph must be in cover set
    for (int leaf = 0; leaf < nodeCount(); leaf++)
    {
        auto ns = myGraph.adjacent(leaf);
        if (ns.size() > 1)
            continue;

        // we have a leaf node
        // get node that connects it
        // add to cover set
        int leafcover = *ns.begin();
        myPath.push_back(leafcover);

        // remove all covered links from working graph
        for (int t : work.adjacent(leafcover))
        {
            work.removeLink(leafcover, t);
            work.removeLink(t, leafcover);
        }
    }

    // loop until all links are covered
    while (work.linkCount())
    {
        //std::cout << "work.linkCount() " << work.linkCount() << "\n";
        auto l = work.links().begin();
        int u = l->first.first;
        int v = l->first.second;

        auto sun = work.adjacent(u);
        auto svn = work.adjacent(v);

        // add non leaf nodes on selected link to cover
        if (sun.size() > 1)
        {
            myPath.push_back(u);
        }
        if (svn.size() > 1)
        {
            myPath.push_back(v);
        }

        // remove all links that can be seen from new cover nodes
        for (int t : sun)
        {
            work.removeLink(u, t);
            work.removeLink(t, u);
        }
        for (int t : svn)
        {
            work.removeLink(v, t);
            work.removeLink(t, v);
        }
    }
    for (int n : myPath)
        myGraph.node(n).myColor = "red";
    myPathCost = -1;
}

void cPathFinder::cliques()
{
    // working copy on input graph
    auto work = myGraph;

    // store for maximal clique collection
    std::vector<std::vector<int>> vclique;

    while (1)
    {
        std::vector<int> clique;

        while (work.nodeCount())
        {
            //std::cout << "work.nodeCount " << work.nodeCount() << " " << clique.size() << "\n";
            if (!clique.size())
            {
                // start by moving an arbitrary node to the clique from the work graph
                auto nit = work.nodes().begin();
                clique.push_back(nit->first);
                work.removeNode(nit->first);
                continue;
            }
            bool found = false;

            // loop over nodes remaining in work graph
            for (auto &u : work.nodes())
            {
                // loop over nodes in clique
                for (int v : clique)
                {
                    if (work.includes_link(v, u.first) ||
                        work.includes_link(u.first, v))
                    {
                        // found node in work that is connected to clique nodes.
                        // move it to clique
                        //std::cout << "add " << myGraph.name(u.first) << "\n";
                        clique.push_back(u.first);
                        work.removeNode(u.first);
                        found = true;
                        break;
                    }
                }
                if (found)
                    break; // found a node to add to clique
            }
            if (!found)
                break; // no more nodes can be added, the clique is maximal
        }

        if (!clique.size())
            break; // did not find a clique

        // add to collection of maximal cliques
        vclique.push_back(clique);
    }

    // Display results
    std::stringstream ss;
    for (auto &c : vclique)
    {
        ss << "clique: ";
        for (int n : c)
            ss << myGraph.name(n) << " ";
        ss << "\n";
    }
    myResults = ss.str();
    std::cout << myResults;
}

void cPathFinder::flows()
{
    /*
    use link cost as maximum capacity
    use link value as used capacity
    use path cost as total flow
    */

    int totalFlow = 0;

    graph::cGraph bkup = myGraph;

    while (1)
    {
        // find path
        path();
        if (!myPath.size())
            break;

        // maximum flow through path
        int maxflow = INT_MAX;
        int u = -1;
        int v;
        for (int v : myPath)
        {
            if (u >= 0)
            {
                double cap = myGraph.findLink(u, v).myCost;
                if (cap < maxflow)
                {
                    maxflow = cap;
                }
            }

            u = v;
        }

        // consume capacity
        u = -1;
        for (int v : myPath)
        {
            if (u >= 0)
            {
                auto &l = myGraph.findLink(u, v);
                l.myCost -= maxflow;
                if (l.myCost < 0.01)
                    myGraph.removeLink(u, v);
                bkup.findLink(u,v).myValue += maxflow;
                bkup.findLink(v,u).myValue += maxflow;
            }
            u = v;
        }

        totalFlow += maxflow;
    }

    myGraph = bkup;

    myPathCost = totalFlow;
    myResults = "total flow " + std::to_string( totalFlow );
    std::cout << myResults << "\n";

            std::stringstream ss;
            for (auto &n : myGraph.nodes() )
            {
                for (auto &l : n.second.myLink)
                {
                        if( n.first > l.first )
                            continue;
                    ss << n.second.myName << " -- "
                       << myGraph.name(l.first) << " capacity "
                       << l.second.myCost << " used "
                       << l.second.myValue << "\n";
                }
            }
            std::cout << ss.str();
}