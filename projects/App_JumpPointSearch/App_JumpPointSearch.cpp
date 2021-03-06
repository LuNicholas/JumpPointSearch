//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_JumpPointSearch.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EJPS.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EBFS.h"

using namespace Elite;

//Destructor
App_PathfindingJPS::~App_PathfindingJPS()
{
	SAFE_DELETE(m_pGridGraph);
}

//Functions
void App_PathfindingJPS::Start()
{
	//Set Camera
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(39.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(73.0f, 35.0f));
	//DEBUGRENDERER2D->GetActiveCamera()->SetMoveLocked(true);
	//DEBUGRENDERER2D->GetActiveCamera()->SetZoomLocked(true);

	//Create Graph
	MakeGridGraph();

	startPathIdx = 0;
	endPathIdx = 7;
}

void App_PathfindingJPS::Update(float deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);

	//INPUT
	bool const middleMousePressed = INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle);
	if (middleMousePressed)
	{
		MouseData mouseData = { INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eMiddle) };
		Elite::Vector2 mousePos = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ (float)mouseData.X, (float)mouseData.Y });

		//Find closest node to click pos
		int closestNode = m_pGridGraph->GetNodeFromWorldPos(mousePos);
		if (m_StartSelected)
		{
			startPathIdx = closestNode;
			m_UpdatePath = true;
		}
		else
		{
			endPathIdx = closestNode;
			m_UpdatePath = true;
		}
	}

	//GRID INPUT
	bool hasGridChanged = m_GraphEditor.UpdateGraph(m_pGridGraph);
	if (hasGridChanged)
	{
		m_UpdatePath = true;
	}

	//IMGUI
	UpdateImGui();


	//CALCULATEPATH
	//If we have nodes and the target is not the startNode, find a path!
	if (m_UpdatePath
		&& startPathIdx != invalid_node_index
		&& endPathIdx != invalid_node_index
		&& startPathIdx != endPathIdx)
	{
		//BFS Pathfinding
		//auto pathfinder = BFS<GridTerrainNode, GraphConnection>(m_pGridGraph);
		auto startNode = m_pGridGraph->GetNode(startPathIdx);
		auto endNode = m_pGridGraph->GetNode(endPathIdx);

		auto pathfinder = JPS<GridTerrainNode, GraphConnection>(m_pGridGraph, m_pHeuristicFunction);
		auto jumpPoints = pathfinder.FindPath(startNode, endNode);

		auto aStarPathfinder = AStar<GridTerrainNode, GraphConnection>(m_pGridGraph, m_pHeuristicFunction);
		std::vector<GridTerrainNode*> JumpPath;
		m_vPath.clear();
		if (jumpPoints.size() >= 2)
		{
			for (int idx{ 0 }; idx < jumpPoints.size() - 1; idx++)
			{
				JumpPath = aStarPathfinder.FindPath(jumpPoints.at(idx), jumpPoints.at(idx + 1));
				m_vPath.insert(m_vPath.end(), JumpPath.begin(), JumpPath.end());
			}
		}

		m_vJumpPoints = pathfinder.GetJumpPoints();

		m_UpdatePath = false;
		std::cout << "New Path Calculated" << std::endl;
	}
}

void App_PathfindingJPS::Render(float deltaTime) const
{
	UNREFERENCED_PARAMETER(deltaTime);
	//Render grid
	m_GraphRenderer.RenderGraph(
		m_pGridGraph,
		m_bDrawGrid,
		m_bDrawNodeNumbers,
		m_bDrawConnections,
		m_bDrawConnectionsCosts
	);

	//Render start node on top if applicable
	if (startPathIdx != invalid_node_index)
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, { m_pGridGraph->GetNode(startPathIdx) }, START_NODE_COLOR);
	}

	//Render end node on top if applicable
	if (endPathIdx != invalid_node_index)
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, { m_pGridGraph->GetNode(endPathIdx) }, END_NODE_COLOR);
	}

	//render path below if applicable
	if (m_vPath.size() > 0)
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, m_vPath);
	}

	//render jumpPoints
	Elite::Color color{ 0,0,0 };
	for(GridTerrainNode* node : m_vJumpPoints)
	{
		m_GraphRenderer.RenderRectNode(m_pGridGraph->GetNodeWorldPos(node), "", 14, color);
	}

}

void App_PathfindingJPS::MakeGridGraph()
{
	m_pGridGraph = new GridGraph<GridTerrainNode, GraphConnection>(
		COLUMNS,
		ROWS,
		m_SizeCell,
		false, //directional or not
		true, // allow diangonal connections or not
		1.f, //default cost for perpendicular connections
		1.5f); //cost for diagonal connections

	//m_pGridGraph->IsolateNode(6);
	//m_pGridGraph->GetNode(7)->SetTerrainType(TerrainType::Mud);
}

void App_PathfindingJPS::UpdateImGui()
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		int menuWidth = 115;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: target");
		ImGui::Text("RMB: start");
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		ImGui::Text("A* Pathfinding");
		ImGui::Spacing();

		ImGui::Text("Middle Mouse");
		ImGui::Text("controls");
		std::string buttonText{ "" };
		if (m_StartSelected)
			buttonText += "Start Node";
		else
			buttonText += "End Node";

		if (ImGui::Button(buttonText.c_str()))
		{
			m_StartSelected = !m_StartSelected;
		}

		ImGui::Checkbox("Grid", &m_bDrawGrid);
		ImGui::Checkbox("NodeNumbers", &m_bDrawNodeNumbers);
		ImGui::Checkbox("Connections", &m_bDrawConnections);
		ImGui::Checkbox("Connections Costs", &m_bDrawConnectionsCosts);
		//if (ImGui::Combo("", &m_SelectedHeuristic, "Manhattan\0Euclidean\0SqrtEuclidean\0Octile\0Chebyshev", 4))
		//{
		//	switch (m_SelectedHeuristic)
		//	{
		//	case 0:
		//		m_pHeuristicFunction = HeuristicFunctions::Manhattan;
		//		break;
		//	case 1:
		//		m_pHeuristicFunction = HeuristicFunctions::Euclidean;
		//		break;
		//	case 2:
		//		m_pHeuristicFunction = HeuristicFunctions::SqrtEuclidean;
		//		break;
		//	case 3:
		//		m_pHeuristicFunction = HeuristicFunctions::Octile;
		//		break;
		//	case 4:
		//		m_pHeuristicFunction = HeuristicFunctions::Chebyshev;
		//		break;
		//	default:
		//		m_pHeuristicFunction = HeuristicFunctions::Chebyshev;
		//		break;
		//	}
		//}
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}
