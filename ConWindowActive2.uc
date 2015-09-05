//=============================================================================
// ConWindowActive
//
// Used for third-person, interactive conversations with the PC involved.
//=============================================================================
class ConWindowActive2 extends ConWindowActive;

function CalculateWindowSizes()
{
	local float lowerHeight;
	local float upperHeight;
	local float lowerCurrentPos;
	local float upperCurrentPos;
	local float recWidth, recHeight;
	local float cinHeight;
	local float ratio;
	local RootWindow root;
	local float minLowerHeight; //MKE

	root = GetRootWindow();

	// Determine the height of the convo windows, based on available space
	if (bForcePlay)
	{
		// calculate the correct 16:9 ratio
		//ratio = 0.5625 * (root.width / root.height);
		//cinHeight = root.height * ratio;
		//
		//upperCurrentPos = 0;
		//upperHeight     = int(0.5 * (root.height - cinHeight));
		//lowerCurrentPos = upperHeight + cinHeight;
		//lowerHeight     = upperHeight;
		//
		//// make sure we don't invert the letterbox if the screen size is strange
		//if (upperHeight < 0)
		//	root.ResetRenderViewport();
		//else
		//	root.SetRenderViewport(0, upperHeight, width, cinHeight);

		//MKE
		minLowerHeight = int(height * lowerFinalHeightPercent); //Taken from 'normal' convo

		cinHeight = min(root.height - minLowerHeight, root.width * 0.5625);
		upperCurrentPos = 0;
		upperHeight     = int(0.5 * (root.height - cinHeight));

		lowerCurrentPos = upperHeight + cinHeight;
		lowerHeight     = upperHeight;

		root.SetRenderViewport(0, upperHeight, width, cinHeight);
	}
	else
	{
		upperHeight = int(height * upperFinalHeightPercent);
		lowerHeight = int(height * lowerFinalHeightPercent);

		// Compute positions for the convo windows
		lowerCurrentPos = int(height - (lowerHeight*currentWindowPos));
		upperCurrentPos = int(-upperHeight * (1.0-currentWindowPos));

		// Squeeze the rendered area
		if (root != None)
			root.SetRenderViewport(0, upperCurrentPos+upperHeight,
		                       width, lowerCurrentPos-(upperCurrentPos+upperHeight));
	}

	// Configure conversation windows
	if (upperConWindow != None)
		upperConWindow.ConfigureChild(0, upperCurrentPos, width, upperHeight);
	if (lowerConWindow != None)
		lowerConWindow.ConfigureChild(0, lowerCurrentPos, width, lowerHeight);

	// Configure Received Window
	if (winReceived != None)
	{
		winReceived.QueryPreferredSize(recWidth, recHeight);
		winReceived.ConfigureChild(10, lowerCurrentPos - recHeight - 5, recWidth, recHeight);
	}

	ConfigureCameraWindow(lowerCurrentPos);
}
