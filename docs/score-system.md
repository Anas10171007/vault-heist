# Task 0.11 score system

`ScoreSystem` receives the model's optimal step count. Alarm capacity is
`ceil(1.5 * optimalSteps) + 2`, calculated using integers as
`(optimalSteps * 3 + 1) / 2 + 2`.

`addStep` increases both the player's step count and alarm. Penalties follow the
brief: a wrong traversal or invalid heap action adds one alarm, a wrong rotation
adds two alarm and removes 50 points, and a hint lowers the maximum result by
one star. `finish` only completes a run whose alarm is not full.

The two-star check is written directly as
`steps <= optimalSteps * 1.25`, matching the project document. The reusable
`AlarmMeter` widget draws the current alarm with a number as well as colour, so
colour is not the only signal.
