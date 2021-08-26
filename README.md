# ResearchTopicGPP
## jump point search

### What is jump point search alhorithm
Jump point search algorithm is a pathfinding algorithm introduced by Daniel Harabor and Alban Grastien that
finds the shortest path from a start node to an end node on a grid based map.

### How does it work
JPS searches for interesting points called jump points on the grid to then jump from jump point to jump point in order to find the goal.
While looking for jump points JPS searches horizontally and vertically to either find nothing at all or a jump point, if JPS finds a jump point then the jump point is saved and
goes back to looking for other jump points. If nothing is found vertically and horizontally it starts moving diagonal by one before searching again.
After the search for jump points is over from the starting node JPS starts doing the same thing but this time from the jump Points, repeating
this process will find all the jump points needed for the shortest path to the goal node.  
![JPS image](https://cdn.discordapp.com/attachments/261574943957975050/880493815331430460/unknown.png)

### Improving on A*
Unlike A* which has to constantly update the open and closed list and finds symmetrical paths JPS has no problem with this.
while are first it looks like that JPS covers more ground than A* JPS spends more time looking for appopiate nodes and cuts down on time by updating the open and openlist less.
![JPS A* comparison](https://cdn.discordapp.com/attachments/261574943957975050/880496201454207089/unknown.png)

### Finding jump points
Finding jump Points is what takes majority of the searching time because since JPS recursively looks horizontal and vertical until finding a jump point.
But what is considered a jump point?
A jump point is node that that has a forced neighbour, while moving sideways JPS prunes most of the neighbours around it because there are other paths that can more efficiently
reach those neighbours if one its neighbours that normally would be pruned is not pruned this is considered a forced neighbour. 
If looking horizontally or vertically the parent of the node that is found to be a jump point is also considered a jump point

**example forced neighbor moving horizontal**

![JPS force neigbour horintal](https://cdn.discordapp.com/attachments/261574943957975050/880499281268715620/unknown.png)

**example forced neighbor moving diagonal**

![JPS force neigbour diagonal](https://cdn.discordapp.com/attachments/261574943957975050/880499338009268234/unknown.png)

**example code looking for forced neigbour**
```
if (moving horizontal)
{
	if (next node to check is not blocked)
	{
		neighbours.pushback(next node to check)

		if (node below is blocked)
			if (node next to blocked node is not blocked)
				neighbours.pushback(node next to blocked node)
            
		if (node above is blocked)
			if (node next to blocked node is not blocked)
				neighbours.pushback(node next to blocked node)
	}
      
	return neighbours;
}
```

### Reaching the goal
After reaching the goal the path can be found backwards by getting the parents from the jump points actually used and reaching the begin node.

```
if (currentNode == GoalNode)
{
  newNode = currentNode;
  while (newNode.ParentNode != nullptr)
  {
    path.push_back(newNode.pNode);
    newNode = newNode.pParentNode;
  }
  path.push_back(newNode.pNode);
}
```

**Blue nodes are walls**

**black nodes are jumpPoints**

![JPS implementation](https://cdn.discordapp.com/attachments/261574943957975050/880509013811396638/unknown.png)


### References

**Original paper by Daniel Harabor and Alban Grastien**
(https://www.researchgate.net/publication/221603063_Online_Graph_Pruning_for_Pathfinding_On_Grid_Maps)
  
**Explanation on Jump point search with interactable path finding to try out**
(https://zerowidth.com/2013/a-visual-explanation-of-jump-point-search.html)
