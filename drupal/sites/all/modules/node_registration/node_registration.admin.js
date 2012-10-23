
jQuery(function($) {

	$('#node-registration-registrations-settings-form input').keydown(function(e) {
		if ( 13 == e.keyCode ) {
			e.preventDefault();

			// don't submit via the first button
			var submit = $(this.form).find('#edit-submit');
			submit.length && submit.click();
		}
	});

});
