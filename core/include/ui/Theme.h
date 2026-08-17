#ifndef THEME_H
#define THEME_H

#include "ui/UIStyle.h"


namespace Theme {
	inline constexpr PanelStyle DarkCard {
		.fillColor = {24, 26, 32, 240},      // Deep dark slate with slight alpha
		.strokeColor = {48, 52, 64, 255},    // Muted border outline
		.cornerRadius = 12.0f,               // Smooth modern corners
		.strokeWidth = 1.0f
	};

	inline constexpr PanelStyle DarkPanel {
		.fillColor = {24, 26, 32, 240}
	};

	inline constexpr ButtonStyle PrimaryButton {
		// --- Panel States ---
		.normalPanel = {
			.fillColor = {88, 101, 242, 255},    // Crisp Indigo
			.strokeColor = {0, 0, 0, 0},
			.cornerRadius = 8.0f,
			.strokeWidth = 0.0f
		},
		.hoveredPanel = {
			.fillColor = {71, 82, 196, 255},     // Slightly darker Indigo
			.strokeColor = {0, 0, 0, 0},
			.cornerRadius = 8.0f,
			.strokeWidth = 0.0f
		},
		.pressedPanel = {
			.fillColor = {58, 66, 160, 255},     // Deep Indigo
			.strokeColor = {0, 0, 0, 0},
			.cornerRadius = 8.0f,
			.strokeWidth = 0.0f
		},
		.disabledPanel = {
			.fillColor = {50, 54, 62, 128},      // Dimmed greyed-out
			.strokeColor = {0, 0, 0, 0},
			.cornerRadius = 8.0f,
			.strokeWidth = 0.0f
		},

		// --- Text States ---
		.normalText = {
			.font = FontId::Bahnschrift,
			.fontSize = 20.f,
			.color = Color::White,
			.alignHorizontal = TextAlignHorizontal::Centre,
			.alignVertical = TextAlignVertical::Middle
		},
		.hoveredText = {
			.font = FontId::Bahnschrift,
			.fontSize = 20.f,
			.color = Color::White,
			.alignHorizontal = TextAlignHorizontal::Centre,
			.alignVertical = TextAlignVertical::Middle
		},
		.pressedText = {
			.font = FontId::Bahnschrift,
			.fontSize = 20.f,
			.color = {220, 220, 220, 255},       // Slightly dimmed white
			.alignHorizontal = TextAlignHorizontal::Centre,
			.alignVertical = TextAlignVertical::Middle
		},
		.disabledText = {
			.font = FontId::Bahnschrift,
			.fontSize = 20.f,
			.color = {140, 140, 140, 255},       // Muted grey
			.alignHorizontal = TextAlignHorizontal::Centre,
			.alignVertical = TextAlignVertical::Middle
		}
	};

	inline constexpr PanelStyle GlassSurface {
		.fillColor = {18, 20, 26, 180},      // Translucent dark background
		.strokeColor = {255, 255, 255, 30},  // Soft semi-transparent white highlight
		.cornerRadius = 12.0f,
		.strokeWidth = 1.0f
	};

	inline constexpr ButtonStyle SecondaryOutline {
		// --- Panel States ---
		.normalPanel = {
			.fillColor = {255, 255, 255, 12},     // 5% White Glass
			.strokeColor = {255, 255, 255, 45},   // Subtle border
			.cornerRadius = 6.0f,
			.strokeWidth = 1.0f
		},
		.hoveredPanel = {
			.fillColor = {255, 255, 255, 25},     // 10% White Glass
			.strokeColor = {255, 255, 255, 120},  // Glowing border
			.cornerRadius = 6.0f,
			.strokeWidth = 1.0f
		},
		.pressedPanel = {
			.fillColor = {255, 255, 255, 40},     // 15% White Glass
			.strokeColor = {255, 255, 255, 180},
			.cornerRadius = 6.0f,
			.strokeWidth = 1.0f
		},
		.disabledPanel = {
			.fillColor = {255, 255, 255, 5},
			.strokeColor = {255, 255, 255, 20},
			.cornerRadius = 6.0f,
			.strokeWidth = 1.0f
		},

		// --- Text States ---
		.normalText = {
			.font = FontId::Bahnschrift,
			.fontSize = 20.f,
			.color = {220, 225, 235, 255},
			.alignHorizontal = TextAlignHorizontal::Centre,
			.alignVertical = TextAlignVertical::Middle
		},
		.hoveredText = {
			.font = FontId::Bahnschrift,
			.fontSize = 20.f,
			.color = Color::White,
			.alignHorizontal = TextAlignHorizontal::Centre,
			.alignVertical = TextAlignVertical::Middle
		},
		.pressedText = {
			.font = FontId::Bahnschrift,
			.fontSize = 20.f,
			.color = {180, 185, 195, 255},
			.alignHorizontal = TextAlignHorizontal::Centre,
			.alignVertical = TextAlignVertical::Middle
		},
		.disabledText = {
			.font = FontId::Bahnschrift,
			.fontSize = 20.f,
			.color = {100, 100, 100, 255},
			.alignHorizontal = TextAlignHorizontal::Centre,
			.alignVertical = TextAlignVertical::Middle
		}
	};

	inline constexpr PanelStyle SuccessBanner {
		.fillColor = {12, 38, 28, 220},      // Deep translucent emerald tint
		.strokeColor = {16, 185, 129, 120},  // Soft green accent outline
		.cornerRadius = 10.0f,
		.strokeWidth = 1.5f
	};

	inline constexpr ButtonStyle SuccessButton {
		.normalPanel = {
			.fillColor = {16, 185, 129, 255},    // Emerald Green
			.strokeColor = {0, 0, 0, 0},
			.cornerRadius = 10.0f,
			.strokeWidth = 0.0f
		},
		.hoveredPanel = {
			.fillColor = {5, 150, 105, 255},     // Darker Emerald
			.strokeColor = {0, 0, 0, 0},
			.cornerRadius = 10.0f,
			.strokeWidth = 0.0f
		},
		.pressedPanel = {
			.fillColor = {4, 120, 87, 255},      // Deep Forest Emerald
			.strokeColor = {0, 0, 0, 0},
			.cornerRadius = 10.0f,
			.strokeWidth = 0.0f
		},
		.disabledPanel = {
			.fillColor = {40, 60, 50, 128},
			.strokeColor = {0, 0, 0, 0},
			.cornerRadius = 10.0f,
			.strokeWidth = 0.0f
		},

		.normalText = {
			.font = FontId::Bahnschrift,
			.fontSize = 40.f,
			.color = Color::White,
			.alignHorizontal = TextAlignHorizontal::Centre,
			.alignVertical = TextAlignVertical::Middle
		},
		.hoveredText = {
			.font = FontId::Bahnschrift,
			.fontSize = 40.f,
			.color = Color::White,
			.alignHorizontal = TextAlignHorizontal::Centre,
			.alignVertical = TextAlignVertical::Middle
		},
		.pressedText = {
			.font = FontId::Bahnschrift,
			.fontSize = 40.f,
			.color = {210, 240, 225, 255},
			.alignHorizontal = TextAlignHorizontal::Centre,
			.alignVertical = TextAlignVertical::Middle
		},
		.disabledText = {
			.font = FontId::Bahnschrift,
			.fontSize = 40.f,
			.color = {120, 140, 130, 255},
			.alignHorizontal = TextAlignHorizontal::Centre,
			.alignVertical = TextAlignVertical::Middle
		}
	};

	inline constexpr TextBoxStyle PrimaryTextBox {
        // --- Panel States ---
        .normalPanel = {
            .fillColor = {14, 16, 20, 255},      // Very dark inset background
            .strokeColor = {48, 52, 64, 255},    // Muted border matches DarkCard
            .cornerRadius = 8.0f,
            .strokeWidth = 1.0f
        },
        .hoveredPanel = {
            .fillColor = {18, 20, 26, 255},      // Slightly lighter on hover
            .strokeColor = {70, 76, 92, 255},    // Brighter border on hover
            .cornerRadius = 8.0f,
            .strokeWidth = 1.0f
        },
        .focusedPanel = {
            .fillColor = {14, 16, 20, 255},      // Keep inset background
            .strokeColor = {88, 101, 242, 255},  // Crisp Indigo focus ring
            .cornerRadius = 8.0f,
            .strokeWidth = 1.5f                  // Slightly thicker to pop
        },
        .disabledPanel = {
            .fillColor = {24, 26, 32, 128},      // Dimmed greyed-out
            .strokeColor = {48, 52, 64, 128},
            .cornerRadius = 8.0f,
            .strokeWidth = 1.0f
        },

        // --- Text States ---
        // Note: Text inputs usually want Left alignment instead of Centre
        .normalText = {
            .font = FontId::CourierNew,
            .fontSize = 20.f,
            .color = {220, 225, 235, 255},
            .alignHorizontal = TextAlignHorizontal::Centre,
            .alignVertical = TextAlignVertical::Middle
        },
        .hoveredText = {
            .font = FontId::CourierNew,
            .fontSize = 20.f,
            .color = Color::White,
            .alignHorizontal = TextAlignHorizontal::Centre,
            .alignVertical = TextAlignVertical::Middle
        },
        .focusedText = {
            .font = FontId::CourierNew,
            .fontSize = 20.f,
            .color = Color::White,
            .alignHorizontal = TextAlignHorizontal::Centre,
            .alignVertical = TextAlignVertical::Middle
        },
        .disabledText = {
            .font = FontId::CourierNew,
            .fontSize = 20.f,
            .color = {100, 100, 100, 255},       // Muted grey
            .alignHorizontal = TextAlignHorizontal::Centre,
            .alignVertical = TextAlignVertical::Middle
        },

        // --- Selection / Cursor ---
		.cursor = {},
        .highlight = { .fillColor = {88, 101, 242, 140} }    // Translucent Indigo for text selection
    };

	inline constexpr SegmentedControlStyle PrimarySegmentedControl {
	    .track = {
	        .fillColor = {14, 16, 20, 255},       // Dark inset background matching PrimaryTextBox
	        .strokeColor = {48, 52, 64, 255},     // Muted border outline matching DarkCard
	        .cornerRadius = 8.0f,
	        .strokeWidth = 1.0f
	    },
	    .selectedOption = {
	        // --- Panel States ---
	        .normalPanel = {
	            .fillColor = {88, 101, 242, 255},  // Crisp Indigo matching PrimaryButton
	            .strokeColor = {0, 0, 0, 0},
	            .cornerRadius = 6.0f,
	            .strokeWidth = 0.0f
	        },
	        .hoveredPanel = {
	            .fillColor = {71, 82, 196, 255},   // Slightly darker Indigo
	            .strokeColor = {0, 0, 0, 0},
	            .cornerRadius = 6.0f,
	            .strokeWidth = 0.0f
	        },
	        .pressedPanel = {
	            .fillColor = {58, 66, 160, 255},   // Deep Indigo
	            .strokeColor = {0, 0, 0, 0},
	            .cornerRadius = 6.0f,
	            .strokeWidth = 0.0f
	        },
	        .disabledPanel = {
	            .fillColor = {50, 54, 62, 128},    // Dimmed greyed-out
	            .strokeColor = {0, 0, 0, 0},
	            .cornerRadius = 6.0f,
	            .strokeWidth = 0.0f
	        },

	        // --- Text States ---
	        .normalText = {
	            .font = FontId::Bahnschrift,
	            .fontSize = 16.f,
	            .color = Color::White,
	            .alignHorizontal = TextAlignHorizontal::Centre,
	            .alignVertical = TextAlignVertical::Middle
	        },
	        .hoveredText = {
	            .font = FontId::Bahnschrift,
	            .fontSize = 16.f,
	            .color = Color::White,
	            .alignHorizontal = TextAlignHorizontal::Centre,
	            .alignVertical = TextAlignVertical::Middle
	        },
	        .pressedText = {
	            .font = FontId::Bahnschrift,
	            .fontSize = 16.f,
	            .color = {220, 220, 220, 255},
	            .alignHorizontal = TextAlignHorizontal::Centre,
	            .alignVertical = TextAlignVertical::Middle
	        },
	        .disabledText = {
	            .font = FontId::Bahnschrift,
	            .fontSize = 16.f,
	            .color = {140, 140, 140, 255},
	            .alignHorizontal = TextAlignHorizontal::Centre,
	            .alignVertical = TextAlignVertical::Middle
	        }
	    },
	    .option = {
	        // --- Panel States ---
	        .normalPanel = {
	            .fillColor = {0, 0, 0, 0},        // Transparent unselected pill
	            .strokeColor = {0, 0, 0, 0},
	            .cornerRadius = 6.0f,
	            .strokeWidth = 0.0f
	        },
	        .hoveredPanel = {
	            .fillColor = {255, 255, 255, 12}, // Subtle glass hover effect
	            .strokeColor = {0, 0, 0, 0},
	            .cornerRadius = 6.0f,
	            .strokeWidth = 0.0f
	        },
	        .pressedPanel = {
	            .fillColor = {255, 255, 255, 25},
	            .strokeColor = {0, 0, 0, 0},
	            .cornerRadius = 6.0f,
	            .strokeWidth = 0.0f
	        },
	        .disabledPanel = {
	            .fillColor = {0, 0, 0, 0},
	            .strokeColor = {0, 0, 0, 0},
	            .cornerRadius = 6.0f,
	            .strokeWidth = 0.0f
	        },

	        // --- Text States ---
	        .normalText = {
	            .font = FontId::Bahnschrift,
	            .fontSize = 16.f,
	            .color = {220, 225, 235, 255},
	            .alignHorizontal = TextAlignHorizontal::Centre,
	            .alignVertical = TextAlignVertical::Middle
	        },
	        .hoveredText = {
	            .font = FontId::Bahnschrift,
	            .fontSize = 16.f,
	            .color = Color::White,
	            .alignHorizontal = TextAlignHorizontal::Centre,
	            .alignVertical = TextAlignVertical::Middle
	        },
	        .pressedText = {
	            .font = FontId::Bahnschrift,
	            .fontSize = 16.f,
	            .color = {180, 185, 195, 255},
	            .alignHorizontal = TextAlignHorizontal::Centre,
	            .alignVertical = TextAlignVertical::Middle
	        },
	        .disabledText = {
	            .font = FontId::Bahnschrift,
	            .fontSize = 16.f,
	            .color = {100, 100, 100, 255},
	            .alignHorizontal = TextAlignHorizontal::Centre,
	            .alignVertical = TextAlignVertical::Middle
	        }
	    }
	};
}


#endif // THEME_H
