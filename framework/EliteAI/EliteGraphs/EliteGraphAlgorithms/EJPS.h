#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class JPS
	{
	public:
		JPS(GridGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		T_NodeType* Jump(T_NodeType* Parent, int horizontal, int vertical, T_NodeType* pStartNode, T_NodeType* pEndNode) const;
		T_NodeType HorSearch(T_NodeType* Parent, int horizontal, int vertical, T_NodeType* pStartNode, T_NodeType* pEndNode) const;
		//T_NodeType VerSearch(T_NodeType* Parent, int horizontal, int vertical, T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		bool IsNodeBlocked(int col, int row) const;

		GridGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	JPS<T_NodeType, T_ConnectionType>::JPS(GridGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> JPS<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		Jump(pStartNode, 1, 0, pStartNode, pGoalNode);

		return vector<T_NodeType*>();
	}

	template <class T_NodeType, class T_ConnectionType>
	T_NodeType* Elite::JPS<T_NodeType, T_ConnectionType>::Jump(T_NodeType* parent, int horizontal, int vertical, T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		//position of new node
		int connections{ 0 };
		Elite::Vector2 nextPos{ m_pGraph->GetNodePos(parent) + Elite::Vector2{float(horizontal), float(vertical)} };

		if (IsNodeBlocked(int(nextPos.x), int(nextPos.y)))//node is either out of bound or blocked(water)
		{
			return nullptr;
		}


		//the new node found is the endNode
		if (nextPos == m_pGraph->GetNodePos(pEndNode))
			return pEndNode;

		
		
		if (horizontal != 0 && vertical != 0)//current search is a diagonal search
		{

		}
		else if(horizontal != 0)//horizontal search
		{
			Elite::Vector2 currentPos{ m_pGraph->GetNodePos(parent) };

			if (!IsNodeBlocked(int(nextPos.x), int(nextPos.y + 1)))//check if node diagonal up is free 
			{
				if (IsNodeBlocked(int(currentPos.x), int(currentPos.y + 1)))//check if node above is blocked
				{
					return parent;//return the current that it is at 
				}
			}

			if (!IsNodeBlocked(int(nextPos.x), int(nextPos.y - 1)))//check if node diagonal donw is free 
			{
				if (IsNodeBlocked(int(currentPos.x), int(currentPos.y - 1)))//check if node below is blocked
				{
					return parent;//return the current that it is at 
				}
			}

		}
		else//vertical search
		{

		}

		T_NodeType* nextNode{m_pGraph->GetNode(int(nextPos.x), int(nextPos.y))};
		Jump(nextNode, horizontal, vertical, pStartNode, pEndNode);


	}

	template <class T_NodeType, class T_ConnectionType>
	T_NodeType Elite::JPS<T_NodeType, T_ConnectionType>::HorSearch(T_NodeType* Parent, int horizontal, int vertical, T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{

	}

	template <class T_NodeType, class T_ConnectionType>
	bool Elite::JPS<T_NodeType, T_ConnectionType>::IsNodeBlocked(int col, int row) const
	{
		if (!m_pGraph->IsWithinBounds(col, row))//check if the node is in the grid boundaries;
			return true;

		int nodeIdx = m_pGraph->GetIndex(col, row);//get node idx from position
		if (m_pGraph->GetNode(nodeIdx)->GetTerrainType() == TerrainType::Water)//check if node isnt blocked
			return true;

		return false;
	}



	template <class T_NodeType, class T_ConnectionType>
	float Elite::JPS<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}



}