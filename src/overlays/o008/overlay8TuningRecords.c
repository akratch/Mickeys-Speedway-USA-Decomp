#include "game/overlay8_tuning.h"

/* Tier B: the two indexed getter tables select these twenty distinct
 * 344-byte records. Fixed scalar reads and all response/damping accesses
 * establish the shared record contract; exported object identities follow
 * the actual ORT rows. Tier D: field/response roles describe the recovered
 * access model, not original source declarations or character-name mapping.
 * Keep writable module lifetime and all shared-role response samples. */

/* Primary bank, selector 0. */
Overlay8TuningRecord D_overlay_008_F0005710_1863468 = {
    0.65f, /* gravity */
    1.0f, /* mass */
    0.05f, /* levelStep */
    1.0f, /* slopeFactor */
    224.0f, /* steeringCoefficient */
    208.0f, /* steeringLimit */
    0.995f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        12.0f,
        /* Forward response, indices 9..40. */
        0.048f, 0.064f, 0.096f, 0.144f,
        0.176f, 0.192f, 0.192f, 0.176f,
        0.144f, 0.096f, 0.064f, 0.048f,
        0.01f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.99f, 0.98f, 0.97f,
    },
};

/* Primary bank, selector 1. */
Overlay8TuningRecord D_overlay_008_F0005868_18635C0 = {
    0.55f, /* gravity */
    0.75f, /* mass */
    0.05f, /* levelStep */
    0.95f, /* slopeFactor */
    240.0f, /* steeringCoefficient */
    224.0f, /* steeringLimit */
    0.995f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        11.75f,
        /* Forward response, indices 9..40. */
        0.08f, 0.128f, 0.192f, 0.224f,
        0.24f, 0.248f, 0.248f, 0.24f,
        0.224f, 0.192f, 0.16f, 0.08f,
        0.005f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.99f, 0.98f, 0.97f,
    },
};

/* Primary bank, selector 2. */
Overlay8TuningRecord D_overlay_008_F00059C0_1863718 = {
    0.65f, /* gravity */
    1.0f, /* mass */
    0.05f, /* levelStep */
    1.0f, /* slopeFactor */
    224.0f, /* steeringCoefficient */
    208.0f, /* steeringLimit */
    0.995f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        12.0f,
        /* Forward response, indices 9..40. */
        0.048f, 0.064f, 0.096f, 0.144f,
        0.176f, 0.192f, 0.192f, 0.176f,
        0.144f, 0.096f, 0.064f, 0.048f,
        0.01f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.99f, 0.98f, 0.97f,
    },
};

/* Primary bank, selector 3. */
Overlay8TuningRecord D_overlay_008_F0005B18_1863870 = {
    0.55f, /* gravity */
    0.75f, /* mass */
    0.05f, /* levelStep */
    0.95f, /* slopeFactor */
    240.0f, /* steeringCoefficient */
    224.0f, /* steeringLimit */
    0.995f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        11.75f,
        /* Forward response, indices 9..40. */
        0.08f, 0.128f, 0.192f, 0.224f,
        0.24f, 0.248f, 0.248f, 0.24f,
        0.224f, 0.192f, 0.16f, 0.08f,
        0.005f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.99f, 0.98f, 0.97f,
    },
};

/* Primary bank, selector 4. */
Overlay8TuningRecord D_overlay_008_F0005C70_18639C8 = {
    0.75f, /* gravity */
    1.5f, /* mass */
    0.05f, /* levelStep */
    1.05f, /* slopeFactor */
    208.0f, /* steeringCoefficient */
    192.0f, /* steeringLimit */
    0.995f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        12.3f,
        /* Forward response, indices 9..40. */
        0.024f, 0.04f, 0.064f, 0.096f,
        0.128f, 0.144f, 0.16f, 0.16f,
        0.128f, 0.096f, 0.064f, 0.032f,
        0.016f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.99f, 0.98f, 0.97f,
    },
};

/* Primary bank, selector 5. */
Overlay8TuningRecord D_overlay_008_F0005DC8_1863B20 = {
    0.75f, /* gravity */
    1.5f, /* mass */
    0.05f, /* levelStep */
    1.05f, /* slopeFactor */
    208.0f, /* steeringCoefficient */
    192.0f, /* steeringLimit */
    0.995f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        12.3f,
        /* Forward response, indices 9..40. */
        0.024f, 0.04f, 0.064f, 0.096f,
        0.128f, 0.144f, 0.16f, 0.16f,
        0.128f, 0.096f, 0.064f, 0.032f,
        0.016f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.99f, 0.98f, 0.97f,
    },
};

/* Primary bank, selector 6. */
Overlay8TuningRecord D_overlay_008_F0005F20_1863C78 = {
    0.65f, /* gravity */
    1.0f, /* mass */
    0.05f, /* levelStep */
    1.0f, /* slopeFactor */
    224.0f, /* steeringCoefficient */
    208.0f, /* steeringLimit */
    0.995f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        12.0f,
        /* Forward response, indices 9..40. */
        0.048f, 0.064f, 0.096f, 0.144f,
        0.176f, 0.192f, 0.192f, 0.176f,
        0.144f, 0.096f, 0.064f, 0.048f,
        0.01f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.99f, 0.98f, 0.97f,
    },
};

/* Primary bank, selector 7. */
Overlay8TuningRecord D_overlay_008_F0006078_1863DD0 = {
    0.65f, /* gravity */
    1.0f, /* mass */
    0.05f, /* levelStep */
    1.0f, /* slopeFactor */
    224.0f, /* steeringCoefficient */
    208.0f, /* steeringLimit */
    0.995f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        12.0f,
        /* Forward response, indices 9..40. */
        0.048f, 0.064f, 0.096f, 0.144f,
        0.176f, 0.192f, 0.192f, 0.176f,
        0.144f, 0.096f, 0.064f, 0.048f,
        0.01f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.99f, 0.98f, 0.97f,
    },
};

/* Primary bank, selector 8. */
Overlay8TuningRecord D_overlay_008_F00061D0_1863F28 = {
    0.65f, /* gravity */
    1.0f, /* mass */
    0.05f, /* levelStep */
    1.0f, /* slopeFactor */
    224.0f, /* steeringCoefficient */
    208.0f, /* steeringLimit */
    0.995f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        12.0f,
        /* Forward response, indices 9..40. */
        0.048f, 0.064f, 0.096f, 0.144f,
        0.176f, 0.192f, 0.192f, 0.176f,
        0.144f, 0.096f, 0.064f, 0.048f,
        0.01f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.99f, 0.98f, 0.97f,
    },
};

/* Primary bank, selector 9. */
Overlay8TuningRecord D_overlay_008_F0006328_1864080 = {
    0.65f, /* gravity */
    1.0f, /* mass */
    0.1f, /* levelStep */
    1.0f, /* slopeFactor */
    216.0f, /* steeringCoefficient */
    184.0f, /* steeringLimit */
    0.99f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        18.0f,
        /* Forward response, indices 9..40. */
        0.05f, 0.06f, 0.08f, 0.095f,
        0.1f, 0.09f, 0.15f, 0.2f,
        0.2f, 0.2f, 0.15f, 0.16f,
        0.15f, 0.1f, 0.075f, 0.085f,
        0.06f, 0.04f, 0.001f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.99f, 0.98f, 0.97f,
    },
};

/* Secondary bank, selector 0. */
Overlay8TuningRecord D_overlay_008_F0006480_18641D8 = {
    0.65f, /* gravity */
    1.0f, /* mass */
    0.1f, /* levelStep */
    1.0f, /* slopeFactor */
    216.0f, /* steeringCoefficient */
    184.0f, /* steeringLimit */
    0.99f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        18.0f,
        /* Forward response, indices 9..40. */
        0.05f, 0.06f, 0.08f, 0.095f,
        0.1f, 0.09f, 0.15f, 0.2f,
        0.2f, 0.2f, 0.15f, 0.16f,
        0.15f, 0.1f, 0.075f, 0.085f,
        0.06f, 0.04f, 0.001f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.99f, 0.98f, 0.97f,
    },
};

/* Secondary bank, selector 1. */
Overlay8TuningRecord D_overlay_008_F00065D8_1864330 = {
    0.55f, /* gravity */
    0.75f, /* mass */
    0.1f, /* levelStep */
    0.9f, /* slopeFactor */
    256.0f, /* steeringCoefficient */
    240.0f, /* steeringLimit */
    0.995f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        17.0f,
        /* Forward response, indices 9..40. */
        0.2f, 0.25f, 0.3f, 0.35f,
        0.4f, 0.45f, 0.5f, 0.45f,
        0.35f, 0.3f, 0.25f, 0.2f,
        0.15f, 0.1f, 0.075f, 0.06f,
        0.035f, 0.003f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.99f, 0.98f, 0.97f,
    },
};

/* Secondary bank, selector 2. */
Overlay8TuningRecord D_overlay_008_F0006730_1864488 = {
    0.65f, /* gravity */
    1.0f, /* mass */
    0.1f, /* levelStep */
    1.0f, /* slopeFactor */
    216.0f, /* steeringCoefficient */
    184.0f, /* steeringLimit */
    0.99f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        18.0f,
        /* Forward response, indices 9..40. */
        0.05f, 0.06f, 0.08f, 0.095f,
        0.1f, 0.09f, 0.15f, 0.2f,
        0.2f, 0.2f, 0.15f, 0.16f,
        0.15f, 0.1f, 0.075f, 0.085f,
        0.06f, 0.04f, 0.001f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.99f, 0.98f, 0.97f,
    },
};

/* Secondary bank, selector 3. */
Overlay8TuningRecord D_overlay_008_F0006888_18645E0 = {
    0.55f, /* gravity */
    0.75f, /* mass */
    0.1f, /* levelStep */
    0.9f, /* slopeFactor */
    256.0f, /* steeringCoefficient */
    240.0f, /* steeringLimit */
    0.995f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        17.0f,
        /* Forward response, indices 9..40. */
        0.2f, 0.25f, 0.3f, 0.35f,
        0.4f, 0.45f, 0.5f, 0.45f,
        0.35f, 0.3f, 0.25f, 0.2f,
        0.15f, 0.1f, 0.075f, 0.06f,
        0.035f, 0.003f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.99f, 0.98f, 0.97f,
    },
};

/* Secondary bank, selector 4. */
Overlay8TuningRecord D_overlay_008_F00069E0_1864738 = {
    0.75f, /* gravity */
    1.5f, /* mass */
    0.1f, /* levelStep */
    1.1f, /* slopeFactor */
    205.0f, /* steeringCoefficient */
    178.0f, /* steeringLimit */
    0.9875f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        19.0f,
        /* Forward response, indices 9..40. */
        0.015f, 0.022f, 0.036f, 0.052f,
        0.065f, 0.075f, 0.075f, 0.075f,
        0.075f, 0.075f, 0.075f, 0.075f,
        0.075f, 0.075f, 0.05f, 0.04f,
        0.001f, 0.04f, 0.01f, 0.01f,
        0.01f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.99f, 0.98f, 0.97f,
    },
};

/* Secondary bank, selector 5. */
Overlay8TuningRecord D_overlay_008_F0006B38_1864890 = {
    0.75f, /* gravity */
    1.5f, /* mass */
    0.1f, /* levelStep */
    1.1f, /* slopeFactor */
    205.0f, /* steeringCoefficient */
    178.0f, /* steeringLimit */
    0.9875f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        19.0f,
        /* Forward response, indices 9..40. */
        0.015f, 0.022f, 0.036f, 0.052f,
        0.065f, 0.075f, 0.075f, 0.075f,
        0.075f, 0.075f, 0.075f, 0.075f,
        0.075f, 0.075f, 0.05f, 0.04f,
        0.001f, 0.04f, 0.01f, 0.01f,
        0.01f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.99f, 0.98f, 0.97f,
    },
};

/* Secondary bank, selector 6. */
Overlay8TuningRecord D_overlay_008_F0006C90_18649E8 = {
    0.75f, /* gravity */
    1.5f, /* mass */
    0.05f, /* levelStep */
    1.1f, /* slopeFactor */
    240.0f, /* steeringCoefficient */
    224.0f, /* steeringLimit */
    0.995f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        18.65f,
        /* Forward response, indices 9..40. */
        0.016f, 0.026f, 0.04f, 0.056f,
        0.084f, 0.102f, 0.118f, 0.132f,
        0.146f, 0.162f, 0.188f, 0.2f,
        0.208f, 0.212f, 0.212f, 0.208f,
        0.2f, 0.2f, 0.2f, 0.2f,
        0.2f, 0.2f, 0.2f, 0.2f,
        0.2f, 0.08f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.978f, 0.975f, 0.975f,
    },
};

/* Secondary bank, selector 7. */
Overlay8TuningRecord D_overlay_008_F0006DE8_1864B40 = {
    0.65f, /* gravity */
    1.0f, /* mass */
    0.05f, /* levelStep */
    1.0f, /* slopeFactor */
    280.0f, /* steeringCoefficient */
    240.0f, /* steeringLimit */
    0.999f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        18.0f,
        /* Forward response, indices 9..40. */
        0.4f, 0.5f, 0.6f, 0.65f,
        0.7f, 0.7f, 0.7f, 0.7f,
        0.7f, 0.7f, 0.7f, 0.7f,
        0.7f, 0.7f, 0.7f, 0.7f,
        0.7f, 0.175f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.925f, 0.925f, 0.925f,
    },
};

/* Secondary bank, selector 8. */
Overlay8TuningRecord D_overlay_008_F0006F40_1864C98 = {
    0.55f, /* gravity */
    0.75f, /* mass */
    0.05f, /* levelStep */
    0.9f, /* slopeFactor */
    350.0f, /* steeringCoefficient */
    349.0f, /* steeringLimit */
    0.9999f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        18.5f,
        /* Forward response, indices 9..40. */
        0.2f, 0.25f, 0.3f, 0.35f,
        0.4f, 0.45f, 0.5f, 0.45f,
        0.35f, 0.3f, 0.25f, 0.2f,
        0.15f, 0.1f, 0.075f, 0.06f,
        0.035f, 0.003f, 0.001f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.925f, 0.925f, 0.925f,
    },
};

/* Secondary bank, selector 9. */
Overlay8TuningRecord D_overlay_008_F0007098_1864DF0 = {
    0.75f, /* gravity */
    1.5f, /* mass */
    0.0f, /* levelStep */
    1.1f, /* slopeFactor */
    100.0f, /* steeringCoefficient */
    99.0f, /* steeringLimit */
    0.9875f, /* turnMultiplier */
    6.0f, /* reverseCap */
    {
        /* Reverse response, indices 0..7. */
        0.1f, 0.125f, 0.15f, 0.175f,
        0.2f, 0.2f, 0.0f, 0.0f,
        /* Speed base / reverse response index 8. */
        24.0f,
        /* Forward response, indices 9..40. */
        0.015f, 0.02f, 0.03f, 0.03f,
        0.03f, 0.03f, 0.03f, 0.03f,
        0.03f, 0.03f, 0.028f, 0.027f,
        0.026f, 0.025f, 0.024f, 0.023f,
        0.022f, 0.01f, 0.008f, 0.007f,
        0.005f, 0.004f, 0.003f, 0.002f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        /* Coast threshold / forward endpoint, index 41. */
        0.01f,
        /* Braking response / forward continuation, indices 42..73. */
        0.14f, 0.15f, 0.16f, 0.17f,
        0.18f, 0.19f, 0.2f, 0.2f,
        0.21f, 0.21f, 0.21f, 0.21f,
        0.2f, 0.2f, 0.19f, 0.18f,
        0.17f, 0.16f, 0.15f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        0.14f, 0.14f, 0.14f, 0.14f,
        /* Damping / positive-response continuation, indices 74..77. */
        1.0f, 0.99f, 0.98f, 0.97f,
    },
};
