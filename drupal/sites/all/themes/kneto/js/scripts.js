(function ($) {

  $(document).ready(function() {
	
	// Colorbox for videos  
	$('.field-name-field-video a').colorbox({iframe:true, innerWidth:620, innerHeight:580});
	
	
	// Placeholder for browsers that don't support the placeholder attribute
	$.support.placeholder = false;
	test = document.createElement('input');
	if('placeholder' in test) $.support.placeholder = true;
	
	if(!$.support.placeholder) {
		var editName = $('#edit-name');
		var editPass = $('#edit-pass');
		var namePlaceholder = $('#edit-name').attr('placeholder');
		var passPlaceholder = $('#edit-pass').attr('placeholder');
		var nameLabel = '<label id="name-placeholder" class="placeholder">'+namePlaceholder+'</label>';
		var passLabel = '<label id="pass-placeholder" class="placeholder">'+passPlaceholder+'</label>';
		
		editName.after(nameLabel);
		editPass.after(passLabel);
		
		editName.focus(function() {
			$('#name-placeholder').hide();
		});
		editName.blur(function() {
			if (this.value == '') {
				$('#name-placeholder').show();
			}
		});
		editPass.focus(function() {
			$('#pass-placeholder').hide();
		});
		editPass.blur(function() {
			if (this.value == '') {
				$('#pass-placeholder').show();
			}
		});
	}
		  
  });

}) (jQuery);