#include "Game.h"
#include"AVL.h"
#include"MaxHeap.h"
#include"Act1.h"
#include<SFML/Graphics.hpp>
using namespace sf;
int main()
{
    Game game;
    game.run();

    return 0;
}
















//using namespace sf;
//int main() {
//	TreeNode* root = new TreeNode(1,50);
//
//	root->left = new TreeNode(2, 25);
//	root->right = new TreeNode(3, 75);
//
//	root->left->left = new TreeNode(4, 10);
//	root->left->right = new TreeNode(5, 30);
//
//	root->right->left = new TreeNode(6, 60);
//	root->right->right = new TreeNode(7, 90);
//
//	root->left->right->left = new TreeNode(8, 28);
//	root->left->right->right = new TreeNode(9,21);
//	root->left->left->left = new TreeNode(10,34);
//
//	TreeNode* root2 = new TreeNode(1, 50);
//
//	root2->left = new TreeNode(2, 25);
//	root2->right = new TreeNode(3, 75);
//
//	root2->left->left = new TreeNode(4, 10);
//	root2->left->right = new TreeNode(5, 30);
//
//	root2->right->left = new TreeNode(6, 60);
//	root2->right->right = new TreeNode(7, 90);
//
//	root2->left->right->left = new TreeNode(8, 28);
//	root2->left->right->right = new TreeNode(9, 21);
////	root2->left->left->left = new TreeNode(10, 34);
//
//	
//	EventLog log;
//
//	BinaryTree* tt = new BinaryTree;
//	tt->setTree(root);
//
//	Act1 vis(tt);
//	vis.setEventLog(&log);
//	vis.runTraversal(7);
//	Clock clock;
//	vis.setMission(Missions::MISSION_1_4);
//	vis.startMission();
//	vis.createSampleTarget();
//	vis.createPlayerTree();
//	RenderWindow window(VideoMode({1920,1080}),"Testing");
//	while (window.isOpen()) {
//		while (const optional event = window.pollEvent()) {
//			if(event->is<Event::Closed>()) window.close();
//			if (const auto* key = event->getIf<sf::Event::KeyPressed>())
//			{
//
//			if(!vis.isMissionActive()){
//				if (key->code == sf::Keyboard::Key::Space)
//					vis.togglePlayback();
//
//				if (key->code == sf::Keyboard::Key::R)
//					vis.restart();
//
//				if (key->code == sf::Keyboard::Key::Right)
//					vis.nextEvent();
//
//				if (key->code == sf::Keyboard::Key::Left)
//					vis.previousEvent();
//				}
//
//				if (key->code == sf::Keyboard::Key::C)
//					vis.resetCamera();
//
//				if(!vis.isMissionActive()){
//				if (key->code == sf::Keyboard::Key::Num1)
//					vis.runTraversal(1);
//
//				if (key->code == sf::Keyboard::Key::Num2)
//					vis.runTraversal(2);
//
//				if (key->code == sf::Keyboard::Key::Num3)
//					vis.runTraversal(3);
//
//				if (key->code == sf::Keyboard::Key::Num4)
//					vis.runTraversal(4);
//
//				if (key->code == sf::Keyboard::Key::Num5)
//					vis.runTraversal(5);
//
//				if (key->code == sf::Keyboard::Key::Num6)
//					vis.runTraversal(6);
//
//				if (key->code == sf::Keyboard::Key::Num7)
//					vis.runTraversal(7);
//
//				if (key->code == sf::Keyboard::Key::Num8)
//					vis.runTraversal(8);
//					}
//
//				if (key->code == sf::Keyboard::Key::M)
//				{
//					if (vis.isMissionActive() &&
//						vis.getMission() == Missions::MISSION_1_5)
//					{
//						vis.swapMirrorNode();
//					}
//					else if (!vis.isMissionActive())
//					{
//						vis.mirrorTree();
//					}
//				}
//
//				if (key->code == sf::Keyboard::Key::P)
//					vis.showPathToSelectedNode();
//
//				if (key->code == sf::Keyboard::Key::L)
//					vis.toggleLCAMode();
//
//				if (key->code == sf::Keyboard::Key::S)
//					vis.checkSymmetry();
//
//				if (key->code == sf::Keyboard::Key::T)
//					vis.compareTree(root2);
//
//					vis.handleInputKey(key->code);
//
//					if (key->code == sf::Keyboard::Key::N)
//						vis.createNewTree();
//
//					if (key->code == sf::Keyboard::Key::R)
//						vis.startRootInput();
//
//					if (key->code == sf::Keyboard::Key::A)
//					{
//						if (vis.isMissionActive() &&
//							vis.getMission() == Missions::MISSION_1_5)
//						{
//							vis.autoMirror();
//						}
//						else if (!vis.isMissionActive())
//						{
//							vis.startChildInput();
//						}
//					}
//
//					if (key->code == sf::Keyboard::Key::Enter)
//					{
//						if (vis.isMissionActive() &&
//							vis.getMission() == Missions::MISSION_1_1)
//						{
//							vis.submitTraversalAnswer();
//						}
//						else if (vis.getMission() == Missions::MISSION_1_2) {
//							vis.submitReconstruction();
//						}
//						else if (vis.getMission() == Missions::MISSION_1_3)
//						{
//							vis.submitHeightInput();
//						}
//						else if (vis.getMission() == Missions::MISSION_1_4)
//						{
//							if (!vis.isLCAPhaseComplete())
//								vis.submitLCAAnswer();
//							else
//								vis.submitPlayerPath();
//						}
//						else if (vis.getMission() == Missions::MISSION_1_5)
//						{
//							vis.submitMirrorAnswer();
//						}
//						else {
//							vis.findLCA();
//						}
//					}
//			}
//
//			if (const auto* mouse = event->getIf<sf::Event::MouseButtonReleased>())
//			{
//				if (mouse->button == sf::Mouse::Button::Left)
//				{
//					sf::Vector2f mousePosition(
//						static_cast<float>(mouse->position.x),
//						static_cast<float>(mouse->position.y)
//					);
//					
//					if (vis.isMissionActive() &&
//						vis.getMission() == Missions::MISSION_1_2)
//					{
//						vis.dropToken(mousePosition, window);
//					}
//					else
//					{
//						vis.stopDragging();
//					}
//				}
//			}
//
//			if (const auto* mouse = event->getIf<sf::Event::MouseMoved>())
//			{
//				sf::Vector2f mousePosition(
//					static_cast<float>(mouse->position.x),
//					static_cast<float>(mouse->position.y)
//				);
//				
//				if (vis.isMissionActive() &&
//					vis.getMission() == Missions::MISSION_1_2)
//				{
//					vis.updateTokenDrag(mousePosition);
//				}
//				else
//				{
//					vis.dragView(mousePosition);
//				}
//			}
//
//			if (const auto* mouse = event->getIf<sf::Event::MouseWheelScrolled>())
//			{
//				vis.zoom(mouse->delta);
//			}
//
//			if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>())
//			{
//				if (mouse->button == sf::Mouse::Button::Left)
//				{
//					sf::Vector2f mousePosition(
//						static_cast<float>(mouse->position.x),
//						static_cast<float>(mouse->position.y)
//					);
//					
//					if (vis.isMissionActive() &&
//						vis.getMission() == Missions::MISSION_1_2)
//					{
//						vis.startTokenDrag(mousePosition);
//					}
//					else
//					{
//						vis.startDragging(mousePosition);
//						vis.selectNode(mousePosition, window);
//					}
//				}
//			}
//
//			if (const auto* text = event->getIf<sf::Event::TextEntered>())
//			{
//				if (vis.isMissionActive() &&
//					vis.getMission() == Missions::MISSION_1_3)
//				{
//					char character = static_cast<char>(
//						event->getIf<sf::Event::TextEntered>()->unicode
//						);
//
//					vis.handleHeightTextInput(character);
//				}
//				else
//				{
//					vis.handleTextInput(
//						event->getIf<sf::Event::TextEntered>()->unicode
//					);
//				}
//			}
//		}
//		float deltaTime = clock.restart().asSeconds();
//		vis.update(deltaTime);
//		window.clear(Color::Black);
//		vis.draw(window);
//		window.display();
//}
//
//}