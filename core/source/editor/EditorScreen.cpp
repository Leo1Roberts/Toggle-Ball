#include "editor/EditorScreen.h"

#include "editor/tool/TransformMode.h"
#include "editor/EditorObstacle.h"
#include "Settings.h"
#include "opengl/Shader.h"
#include "editor/operation/TranslateOperation.h"
#include "ui/Theme.h"
#include "ui/UIContainer.h"
#include "ui/UIList.h"
#include "ui/UISegmentedControl.h"
#include "ui/UITextBox.h"
#include "ui/UIToggle.h"
#include "ui/UiDropDownList.h"


const glm::vec3 groundColor = colorToLinear({76, 76, 76});
const glm::vec3 skyColor = colorToLinear({85, 110, 128});
const glm::vec3 sunColor = colorToLinear({255, 255, 230});

constexpr glm::vec3 upDirection{0, 0, 1};
static glm::vec3 viewUpDirection;
const glm::vec3 sunDirection = normalize(glm::vec3(2, 2, 3));
static glm::vec3 viewSunDirection;


EditorScreen::EditorScreen(std::unique_ptr<LevelDescriptor> levelToEdit, const std::function<void()>& testLevelCallback) :
	scene(
		std::move(levelToEdit),
		[this] { updateEphemeralMeshes(); }),
	transformMode(&scene, &camera),
	shapeMode(&scene, &camera) {
	camera.reset(scene.level->arenaWidth, scene.level->arenaHeight);
	viewUpDirection = camera.getWorldToViewRotationMatrix() * upDirection;
	viewSunDirection = camera.getWorldToViewRotationMatrix() * sunDirection;


	auto layout = uiManager.addNode(std::make_unique<UIHorizontalList>(0.f, 0.f));

	auto mainArea = layout->addChild<UIVerticalList>(0.f, 0.f);

	auto statusBar = mainArea->addChild(std::make_unique<UIPanel>(
		PanelStyle{
			.fillColor = {24, 26, 32, 150},
		}));
	statusBar->setLayout({
		.anchor = Anchor::TopCentre,
		.widthMode  = SizingMode::Stretch,
		.heightMode = SizingMode::Wrap,
		.padding = glm::vec2(4.f)
	});

	toolbar = statusBar->addChild<UIContainer>();
	toolbar->setLayout({
		.anchor = Anchor::Centre,
		.widthMode  = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
		.padding = glm::vec2(4.f)
	});

	viewportUI = mainArea->addChild<UIContainer>();
	operationUI = viewportUI->addChild<UIContainer>();

	auto propertiesPanel = layout->addChild<UIPanel>(Theme::DarkPanel);
	propertiesPanel->setLayout({
		.widthMode  = SizingMode::Absolute, .width = 300.f,
		.heightMode = SizingMode::Stretch
	});
	auto propertiesList = propertiesPanel->addChild<UIVerticalList>(0.f, 0.f);


	auto levelAndBallPropertiesList = propertiesList->addChild<UIVerticalList>(10.f);
	levelAndBallPropertiesList->setLayout({ .padding = glm::vec2(20.f) });

	auto makeListItem = [this](const std::string& labelText, float* property) {
		auto item =  std::make_unique<UIHorizontalList>(10.f, 0.f);
		item->setLayout({
			.widthMode  = SizingMode::Stretch,
			.heightMode = SizingMode::Wrap,
		});
		item->addChild(std::make_unique<UIText>(labelText,
			TextStyle{
				.color = Color::LightGrey,
				.alignVertical = TextAlignVertical::Middle
			}));
		auto textField = item->addChild<UITextBox>(TextInputBuffer::Float, Theme::PrimaryTextBox, "-");
		textField->setLayout({
			.heightMode = SizingMode::Wrap,
			.padding = {0.f, 10.f}
		});
		textField->setOnTextChange([this, property](const UITextBox& tb) {
			if (auto value = tb.getValue<std::optional<float>>())
				*property = *value;
			else
				scene.cancelLevelChange();
		});
		textField->setOnConfirm([this](const UITextBox& tb) {
			if (tb.isEmpty())
				scene.cancelLevelChange();
			else
				scene.commitLevelChange();
		});
		textField->setOnCancel([this](const UITextBox&) { scene.cancelLevelChange(); });
		textField->setValueProvider([property] {
			return floatToString(*property, 3);
		});

		return item;
	};

	levelAndBallPropertiesList->addChild(makeListItem("Arena width", &scene.level->arenaWidth));
	levelAndBallPropertiesList->addChild(makeListItem("Arena height", &scene.level->arenaHeight));
	levelAndBallPropertiesList->addChild(makeListItem("Transition time", &scene.level->transitionTime));
	levelAndBallPropertiesList->addChild(makeListItem("Ball position X", &scene.ball.descriptor->initialPosition.x));
	levelAndBallPropertiesList->addChild(makeListItem("Ball position Y", &scene.ball.descriptor->initialPosition.y));

	obstacleMotionPropertiesList = propertiesList->addChild<UIVerticalList>(10.f);
	obstacleMotionPropertiesList->setLayout({ .padding = glm::vec2(20.f) });

	auto helpBar = mainArea->addChild(std::make_unique<UIPanel>(
		PanelStyle{
			.fillColor = {24, 26, 32, 150},
		}));
	helpBar->setLayout({
		.anchor = Anchor::BottomCentre,
		.widthMode  = SizingMode::Stretch,
		.heightMode = SizingMode::Wrap,
	});

	bindingHints = helpBar->addChild(std::make_unique<UIHorizontalList>(10.f, 0.f));
	bindingHints->setLayout({
		.heightMode = SizingMode::Wrap,
		.padding = glm::vec2(5.f),
	});


	auto stateToggle = statusBar->addChild<UIToggle>(scene.isToggled(),
		ToggleStyle{
			.normalTrackOff = PanelStyle{ .fillColor = Color::StateA, .cornerRadius = std::numeric_limits<float>::max() },
			.hoveredTrackOff = PanelStyle{ .fillColor = Color::StateAHovered, .cornerRadius = std::numeric_limits<float>::max() },
			.normalTrackOn = PanelStyle{ .fillColor = Color::StateB, .cornerRadius = std::numeric_limits<float>::max() },
			.hoveredTrackOn = PanelStyle{ .fillColor = Color::StateBHovered, .cornerRadius = std::numeric_limits<float>::max() },
			.handle = PanelStyle{ .fillColor = Color::White,  .cornerRadius = std::numeric_limits<float>::max() }
		});
	stateToggle->setLayout({
		.anchor = Anchor::CentreLeft,
		.widthMode  = SizingMode::Absolute, .width = 60.f,
		.heightMode = SizingMode::Wrap,
		.padding = glm::vec2(5.f),
		.margin = glm::vec2(5.f)
	});
	stateToggle->setHandleLayout({
		.widthMode  = SizingMode::Absolute, .width  = 20.f,
		.heightMode = SizingMode::Absolute, .height = 20.f,
	});
	stateToggle->setOnToggle([this](bool, byte mods) { scene.toggle(!(mods & MOD_CTRL)); });
	stateToggle->setValueProvider([this] { return scene.getTogglePosition(); });

	auto testButton = statusBar->addChild<UIButton>("Test level", Theme::SecondaryOutline);
	testButton->setLayout({
		.anchor = Anchor::CentreRight,
		.widthMode  = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
		.padding = {8.f, 6.f},
		.margin = glm::vec2(5.f)
	});
	testButton->setOnTrigger(testLevelCallback);

	auto modeSelector = viewportUI->addChild<UIDropDownList>(std::vector<std::string>{"Transform Mode", "Shape Mode"}, 0, Theme::PrimaryDropDownList, 2.f, 5.f, glm::vec2(5.f));
	modeSelector->setLayout({
		.anchor = Anchor::TopLeft,
		.widthMode  = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
		.padding = glm::vec2(8.f),
		.margin = glm::vec2(10.f)
	});
	modeSelector->setOptionLayout({
		.anchor = Anchor::Centre,
		.widthMode  = SizingMode::Stretch,
		.heightMode = SizingMode::Wrap,
		.padding = glm::vec2(5.f),
	});
	modeSelector->setOptionTextLayout({
		.anchor = Anchor::Centre,
		.widthMode  = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
	});
	modeSelector->setOnSelectedOptionChange([this](int selected) {
		switch (selected) {
		case 0:
			selectMode(&transformMode);
			break;
		case 1:
			selectMode(&shapeMode);
			break;
		default:;
		}
	});
	modeSelector->setValueProvider([this] {
		if (currentMode == &transformMode)
			return 0;
		if (currentMode == &shapeMode)
			return 1;
		return -1;
	});

	selectMode(&transformMode);
}


void EditorScreen::processEvent(const Event& event) {
	if (currentMode->hasActiveOperation()) { // Active operation takes priority over UI
		auto response = currentMode->processEvent(event);
		if (response.operationChanged)
			updateDynamicUI();
		if (response.consumedEvent)
			return;

		if (uiManager.processEvent(event))
			return;
	} else {
		if (uiManager.processEvent(event))
			return;

		auto response = currentMode->processEvent(event);
		if (response.operationChanged)
			updateDynamicUI();
		if (response.consumedEvent)
			return;
	}

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			switch (*actionCode) {
			case ActionCode::Undo:
				if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
					currentMode->commitActiveOperation();
					scene.undo();
					updateDynamicUI();
					return;
				} break;
			case ActionCode::Redo:
				if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
					currentMode->cancelActiveOperation();
					scene.redo();
					updateDynamicUI();
					return;
				} break;
			default:;
			}

			if (!currentMode->hasActiveOperation() && key->action == KeyAction::Down) {
				switch (*actionCode) {
				case ActionCode::TransformMode:
					selectMode(&transformMode);
					break;
				case ActionCode::ShapeMode:
					selectMode(&shapeMode);
					break;
				case ActionCode::Toggle:
					scene.toggle();
					break;
				case ActionCode::InstantToggle:
					scene.toggle(false);
					break;
				case ActionCode::SelectAll:
					scene.selectAll();
					scene.commitSelectionChange();
					break;
				case ActionCode::DeselectAll:
					scene.deselectAll();
					scene.commitSelectionChange();
					break;
				default:;
				}
			}
		}
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		switch (pointer->action) {
		case PointerAction::Down:
			if (pointer->button == PointerButton::Tertiary) {
				panning = true;
				camera.startPan(pointer->position);
			} break;
		case PointerAction::Move:
		case PointerAction::Drag:
			if (panning)
				camera.updatePan(pointer->position);
			break;
		case PointerAction::Up:
			if (pointer->button == PointerButton::Tertiary)
				panning = false;
			break;
		case PointerAction::Scroll: {
			float zoomChange = 1.f;
			if (pointer->scroll.y > 0)
				zoomChange = 1.2f;
			else if (pointer->scroll.y < 0)
				zoomChange = 1.f / 1.2f;
			camera.zoom(zoomChange, pointer->position);
			updateEphemeralMeshes();
		}
		default:;
		}
	}
}


void EditorScreen::update(microseconds dt) {
	scene.update(dt);

	auto selectionState = scene.getSelectionState();
	if (cachedSelectionState.obstacles != selectionState.obstacles) {
		updateObstacleMotionPropertiesList();
		cachedSelectionState = selectionState;
	}

	uiManager.update(dt);
}


constexpr glm::mat3 backgroundRotation = {
	0, 0, -1,
	0, 1, 0,
	1, 0, 0
};

void EditorScreen::render() {
	Shaders::object->use();

	Shaders::object->setVec3("uGroundColor", groundColor);
	Shaders::object->setVec3("uSkyColor", skyColor);
	Shaders::object->setVec3("uSunColor", sunColor);
	Shaders::object->setMat4("uProjection", camera.getProjectionMatrix());
	Shaders::object->setVec3("uUpDirection", viewUpDirection);
	Shaders::object->setVec3("uSunDirection", viewSunDirection);

	drawObject(Meshes::plane.get(), Textures::white.get(),
			   {-1.f, 0, scene.level->arenaHeight / 2.f},
			   backgroundRotation,
			   {scene.level->arenaHeight, scene.level->arenaWidth, 1.f});

	Shaders::outline->use();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	for (int i = 0; i < scene.obstacles.size(); i++) {
		const auto& obstacle = scene.obstacles[i];
		if (obstacle.isSelected()) {
			Shaders::outline->setVec4("uOutlineColor", col(obstacle.descriptor->color, Settings::Colors.domainOpacity));

			// Place domains at different depths to prevent Z-fighting. Last part keeps the ball outline on top.
			float depth = ((float)i - (float)scene.obstacles.size()) / (float)scene.obstacles.size();
			glm::vec3 position = {depth, obstacle.getDomainPosition()};
			glm::mat4 worldMatrix = buildScaledWorldMatrix(glm::mat3(1.f), position);
			Shaders::outline->setMat4("uProjectionFull", camera.getProjectionMatrix() * camera.getViewMatrix() * worldMatrix);

			obstacle.getDomainMesh()->draw();
		}
	}
	glEnable(GL_CULL_FACE);

	Shaders::object->use();
	for (const auto& obstacle: scene.obstacles) {
		Shaders::object->setFloat("uAlpha", getObstacleOpacity(obstacle));

		drawObject(obstacle.getObstacleMesh(), Textures::white.get(),
				   obstacle.getKinematicState()->getPosition(),
				   obstacle.getKinematicState()->getRotation());
	}
	glDisable(GL_BLEND);

	glDepthFunc(GL_ALWAYS);
	drawObject(Meshes::ball.get(), scene.ball.getTexture(),
			   planarToWorld(scene.ball.descriptor->initialPosition),
			   glm::mat3(1));


	Shaders::outline->use();
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);

	for (int i = 0; i < scene.obstacles.size(); i++) {
		const auto& obstacle = scene.obstacles[i];
		if (obstacle.isSelected() && (i != scene.selectionFocus.index || scene.selectionFocus.type != EntityType::Obstacle)) {
			glm::vec4 outlineColor = Color::Selected;
			outlineColor.a *= getObstacleOpacity(obstacle);
			Shaders::outline->setVec4("uOutlineColor", outlineColor);

			drawObstacleOutline(obstacle);
		}
	}

	if (scene.selectionFocus.type == EntityType::Obstacle) {
		const auto& obstacle = scene.obstacles[scene.selectionFocus.index];
		if (obstacle.isSelected()) {
			glm::vec4 outlineColor = Color::Focused;
			outlineColor.a *= getObstacleOpacity(obstacle);
			Shaders::outline->setVec4("uOutlineColor", outlineColor);

			drawObstacleOutline(obstacle);
		}
	}

	if (scene.ball.isSelected()) {
		glm::vec4 outlineColor;
		if (scene.selectionFocus.type == EntityType::Ball)
			outlineColor = Color::Focused;
		else
			outlineColor = Color::Selected;
		Shaders::outline->setVec4("uOutlineColor", outlineColor);

		glm::mat4 worldMatrix = buildScaledWorldMatrix(glm::mat3(1.f), planarToWorld(scene.level->ballDescriptor->initialPosition), planarToWorld(glm::vec2(scene.ball.getOutlineRadius())));
		Shaders::outline->setMat4("uProjectionFull", camera.getProjectionMatrix() * camera.getViewMatrix() * worldMatrix);

		Meshes::ball->draw();
	}
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	currentMode->renderGizmos(gizmoRenderer);

	uiManager.render();

	glDisable(GL_BLEND);
}


float EditorScreen::getObstacleOpacity(const EditorObstacle& obstacle) const {
	float opacity = 1.f;

	auto motionSpec = obstacle.descriptor->motion.get();
	if (dynamic_cast<OscillatingPositionSpec*>(motionSpec) ||
	    dynamic_cast<OscillatingAngleSpec*>(motionSpec))
		// Includes a small period where the obstacle is completely invisible
		opacity = 2.5f * std::abs(0.5f - scene.getTogglePosition()) - 0.2f;

	return opacity;
}

void EditorScreen::drawObject(const Mesh<ObjectVertex>* model, const Texture* texture, glm::vec3 position, const glm::mat3& rotation, glm::vec3 scale) const {
	glm::mat4 worldMatrix = buildScaledWorldMatrix(rotation, position, scale);

	glm::mat4 bodyToView = camera.getViewMatrix() * worldMatrix;
	glm::mat3 bodyToViewRotation = camera.getWorldToViewRotationMatrix() * rotation;

	Shaders::object->setMat3("uBodyToViewRot", bodyToViewRotation, false);
	Shaders::object->setMat4("uBodyToView", bodyToView);

	texture->bind(0);
	model->draw();
}

void EditorScreen::drawObstacleOutline(const EditorObstacle& obstacle) const {
	glm::mat4 worldMatrix = buildScaledWorldMatrix(obstacle.getKinematicState()->getRotation(), obstacle.getKinematicState()->getPosition());
	Shaders::outline->setMat4("uProjectionFull", camera.getProjectionMatrix() * camera.getViewMatrix() * worldMatrix);
	obstacle.getOutlineMesh()->draw();

	glDisable(GL_DEPTH_TEST);
	worldMatrix = buildScaledWorldMatrix(glm::mat3(1.f), obstacle.getKinematicState()->getPosition(), glm::vec3(uiToWorldScale * Settings::Sizes.centreDotRadius));
	Shaders::outline->setMat4("uProjectionFull", camera.getProjectionMatrix() * camera.getViewMatrix() * worldMatrix);
	Meshes::ball->draw();
	glEnable(GL_DEPTH_TEST);
}


void EditorScreen::updateEphemeralMeshes() {
	uiToWorldScale = uiManager.getScale() * camera.getHalfHeight() * 2.f / (float)height;
	scene.ball.updateOutlineRadius(uiToWorldScale);
	for (auto& obstacle : scene.obstacles) {
		obstacle.uiToWorldScale = uiToWorldScale;
		obstacle.generateEphemeralMeshes();
	}
}

void EditorScreen::doResize() {
	uiManager.resize(width, height, dpiScale);
	auto viewportBounds = operationUI->getAbsoluteBounds() * uiManager.getScale();
	viewportBounds.y() = (float)height - viewportBounds.height() - viewportBounds.y();
	camera.update((float)width, (float)height, scene.level->arenaWidth, scene.level->arenaHeight, viewportBounds);
	updateEphemeralMeshes();
}


void EditorScreen::updateObstacleMotionPropertiesList() {
	uiManager.removeAllChildrenOfNode(obstacleMotionPropertiesList);

	std::vector<MotionSpecPropertyDescriptor> commonProperties;
	for (const auto& obstacle : scene.obstacles)
		if (obstacle.isSelected()) {
			const auto& properties = obstacle.descriptor->motion->getPropertyDescriptors();

			if (commonProperties.empty())
				commonProperties = properties;
			else {
				std::erase_if(commonProperties, [&](const auto& property) {
					return std::ranges::find(properties, property) == properties.end(); });

				if (commonProperties.empty()) break;
			}
		}

	for (const auto& propertyDescriptor : commonProperties) {
		auto makeListItem = [this, propertyDescriptor](col labelColor, bool toggled = false) {
			auto item =  std::make_unique<UIHorizontalList>(10.f, 0.f);
			item->setLayout({
				.widthMode  = SizingMode::Stretch,
				.heightMode = SizingMode::Wrap,
			});
			item->addChild(std::make_unique<UIText>(getMotionSpecPropertyName(propertyDescriptor.property),
				TextStyle{
					.color = labelColor,
					.alignVertical = TextAlignVertical::Middle
				}));
			auto textField = item->addChild<UITextBox>(TextInputBuffer::Float, Theme::PrimaryTextBox, "-");
			textField->setLayout({
				.widthMode  = SizingMode::Stretch,
				.heightMode = SizingMode::Wrap,
				.padding = {0.f, 10.f}
			});
			textField->setOnFocusGained([this, propertyDescriptor] {
				switch (propertyDescriptor.property) {
				case MotionSpecProperty::AngularSpeed_rpm:
				case MotionSpecProperty::AngularFrequency_opm:
					scene.demonstrateMotion = true;
				default:;
				}
			});
			textField->setOnTextChange([this, propertyDescriptor, toggled](const UITextBox& tb) {
				if (auto value = tb.getValue<std::optional<float>>()) {
					for (auto& obstacle : scene.obstacles)
						if (obstacle.isSelected()) {
							obstacle.setMotionProperty(*value, propertyDescriptor.property, toggled);
							obstacle.initKinematicState();
							obstacle.generateDomainMesh();
						}
				} else
					scene.cancelLevelChange();
			});
			textField->setOnConfirm([this](const UITextBox& tb) {
				if (tb.isEmpty())
					scene.cancelLevelChange();
				else
					scene.commitLevelChange();
				scene.demonstrateMotion = false;
			});
			textField->setOnCancel([this](const UITextBox&) {
				scene.cancelLevelChange();
				scene.demonstrateMotion = false;
			});
			textField->setValueProvider([this, propertyDescriptor, toggled] {
				float value = NAN;
				for (const auto& obstacle : scene.obstacles)
					if (obstacle.isSelected()) {
						float v = obstacle.getMotionProperty(propertyDescriptor.property, toggled);
						if (std::isnan(value))
							value = v;
						else if (v != value) {
							value = NAN;
							break;
						}
					}

				return std::isnan(value) ? "" : floatToString(value, 3);
			});

			return item;
		};

		if (propertyDescriptor.stateful) {
			obstacleMotionPropertiesList->addChild(makeListItem(Color::StateA, false));
			obstacleMotionPropertiesList->addChild(makeListItem(Color::StateB, true));
		} else
			obstacleMotionPropertiesList->addChild(makeListItem(Color::LightGrey));
	}
}


void EditorScreen::selectMode(ToolMode* mode) {
	if (mode == currentMode) return;
	currentMode = mode;
	updateToolbar();
	updateDynamicUI();
}


void EditorScreen::updateDynamicUI() {
	uiManager.removeAllChildrenOfNode(operationUI);
	currentMode->createOperationUI(*operationUI);

	uiManager.removeAllChildrenOfNode(bindingHints);

	std::vector<BindingHint> hints;

	if (auto binding = Settings::Bindings->findBinding(ActionCode::TestOrEditLevel))
		hints.emplace_back(*binding, "Test level");

	hints.append_range(currentMode->getBindingHints());

	for (const auto& hint : hints) {
		auto hintUI = bindingHints->addChild<UIHorizontalList>(2.f, 0.f);
		hintUI->setLayout({
			.anchor = Anchor::Centre,
			.widthMode  = SizingMode::Wrap,
			.heightMode = SizingMode::Wrap,
		});

		TextStyle textStyle = {
			.fontSize = 14.f,
			.color = Color::LightGrey,
			.alignHorizontal = TextAlignHorizontal::Centre,
			.alignVertical = TextAlignVertical::Middle,
		};

		auto addKeySymbol = [&hintUI, textStyle](std::string_view name) {
			auto box = hintUI->addChild<UIPanel>(
				PanelStyle{
					.fillColor = Color::Transparent,
					.strokeColor = Color::LightGrey,
					.cornerRadius = 3.f,
					.strokeWidth = 1.f,
				});
			float paddingY = textStyle.fontSize / 8.f;
			if (name.length() == 1)
				box->setLayout({
					.anchor = Anchor::Centre,
					.widthMode  = SizingMode::Absolute, .width  = textStyle.fontSize + paddingY * 2.f,
					.heightMode = SizingMode::Absolute, .height = textStyle.fontSize + paddingY * 2.f,
				});
			else
				box->setLayout({
					.anchor = Anchor::Centre,
					.widthMode  = SizingMode::Wrap,
					.heightMode = SizingMode::Wrap,
					.padding = {paddingY * 1.75f, paddingY}
				});
			auto text = box->addChild<UIText>(std::string(name), textStyle);
			text->setLayout({
				.anchor = Anchor::Centre,
				.widthMode  = SizingMode::Wrap,
				.heightMode = SizingMode::Wrap,
			});
			return box;
		};

		if (hint.keyChord.modifiers & MOD_CTRL)
			addKeySymbol(KeyRegistry::toString(KeyCode::Ctrl));
		if (hint.keyChord.modifiers & MOD_SHIFT)
			addKeySymbol(KeyRegistry::toString(KeyCode::Shift));
		if (hint.keyChord.modifiers & MOD_ALT)
			addKeySymbol(KeyRegistry::toString(KeyCode::Alt));
		if (hint.keyChord.code != KeyCode::Unknown)
			addKeySymbol(KeyRegistry::toString(hint.keyChord.code));

		auto effectNode = hintUI->addChild(std::make_unique<UIText>(hint.label, textStyle));
		effectNode->setLayout({
			.anchor = Anchor::Centre,
			.widthMode  = SizingMode::Wrap,
			.heightMode = SizingMode::Wrap,
			.margin = {3.f, 0.f}
		});
	}
}

void EditorScreen::updateToolbar() {
	uiManager.removeAllChildrenOfNode(toolbar);
	currentMode->populateToolbar(*toolbar);
}